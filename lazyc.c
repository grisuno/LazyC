/*
 * mini_gcc – minimal C compiler for x86-64 Linux with Hotswap ELF loader.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file includes a self-contained ELF loader (RunELF) that can load
 * position-independent ELF objects (BOFs) into memory and execute them
 * with full symbol resolution, trampolines for far calls, and a safe
 * execution environment. It is designed to be integrated into the
 * compiler for hot‑swapping of compiled modules.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <elf.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <stddef.h>

/*
 * Configuration structure – centralises all tunable parameters.
 */
struct hotswap_config {
    size_t beacon_output_size;
    size_t max_token_len;
    size_t max_symbols;
    size_t max_ident_len;
    size_t max_source_size;
    size_t stack_align;
    size_t max_struct_members;
    size_t max_macros;
    size_t max_cases_per_switch;
    size_t max_strings;
    size_t trampoline_cache_initial;
    size_t trampoline_cache_grow;
    size_t page_size;
};

static const struct hotswap_config g_config = {
    .beacon_output_size     = 8192,
    .max_token_len          = 64,
    .max_symbols            = 2048,
    .max_ident_len          = 32,
    .max_source_size        = 1048576,
    .stack_align            = 16,
    .max_struct_members     = 256,
    .max_macros             = 256,
    .max_cases_per_switch   = 256,
    .max_strings            = 2048,
    .trampoline_cache_initial = 8,
    .trampoline_cache_grow  = 2,
    .page_size              = 0,  /* determined at runtime */
};

/*
 * Global output buffer for BeaconPrintf/BeaconOutput.
 */
static char g_beacon_output[8192];
static size_t g_output_len = 0;

/*
 * Symbol resolver table – pointers to libc and beacon functions.
 */
static void *g_printf_ptr      = NULL;
static void *g_strlen_ptr      = NULL;
static void *g_memcpy_ptr      = NULL;
static void *g_memset_ptr      = NULL;
static void *g_exit_ptr        = NULL;
static void *g_dlsym_ptr       = NULL;
static void *g_dlerror_ptr     = NULL;
static void *g_dlopen_ptr      = NULL;
static void *g_dlclose_ptr     = NULL;
static void *g_write_ptr       = NULL;
static void *g_mmap_ptr        = NULL;
static void *g_munmap_ptr      = NULL;
static void *g_mprotect_ptr    = NULL;
static void *g_BeaconPrintf_ptr = NULL;
static void *g_BeaconOutput_ptr = NULL;
static void *g_socket_ptr      = NULL;
static void *g_connect_ptr     = NULL;
static void *g_inet_addr_ptr   = NULL;
static void *g_htons_ptr       = NULL;
static void *g_send_ptr        = NULL;
static void *g_recv_ptr        = NULL;
static void *g_close_ptr       = NULL;
static void *g_getaddrinfo_ptr = NULL;
static void *g_freeaddrinfo_ptr = NULL;

/*
 * Symbol resolver entry.
 */
typedef struct {
    const char *name;
    void **ptr;
} SymbolResolver;

static SymbolResolver g_external_symbols[] = {
    { "printf",      &g_printf_ptr },
    { "strlen",      &g_strlen_ptr },
    { "memcpy",      &g_memcpy_ptr },
    { "memset",      &g_memset_ptr },
    { "exit",        &g_exit_ptr },
    { "dlsym",       &g_dlsym_ptr },
    { "dlerror",     &g_dlerror_ptr },
    { "dlopen",      &g_dlopen_ptr },
    { "dlclose",     &g_dlclose_ptr },
    { "write",       &g_write_ptr },
    { "mmap",        &g_mmap_ptr },
    { "munmap",      &g_munmap_ptr },
    { "mprotect",    &g_mprotect_ptr },
    { "BeaconPrintf", &g_BeaconPrintf_ptr },
    { "BeaconOutput", &g_BeaconOutput_ptr },
    { "socket",      &g_socket_ptr },
    { "connect",     &g_connect_ptr },
    { "inet_addr",   &g_inet_addr_ptr },
    { "htons",       &g_htons_ptr },
    { "send",        &g_send_ptr },
    { "recv",        &g_recv_ptr },
    { "close",       &g_close_ptr },
    { "getaddrinfo", &g_getaddrinfo_ptr },
    { "freeaddrinfo", &g_freeaddrinfo_ptr },
    { NULL, NULL }
};

/*
 * Trampoline structures for far calls.
 */
typedef struct {
    void *addr;
    size_t size;
} Trampoline;

typedef struct {
    void *original;
    void *trampoline;
} TrampolineCache;

static TrampolineCache *g_trampoline_cache = NULL;
static size_t g_cache_count = 0;
static size_t g_cache_capacity = 0;

static Trampoline *g_trampolines = NULL;
static size_t g_trampolines_count = 0;
static size_t g_trampolines_capacity = 0;

/*
 * Forward declarations.
 */
static void *create_trampoline(void *target);
static void cleanup_trampolines(void);
static void *get_or_create_trampoline(void *target);
static void BeaconPrintf(int type, const char *fmt, ...);
static void BeaconOutput(int type, const char *data, int len);
static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t arglen);
static size_t page_align(size_t size);
static int resolve_external_symbols(void);
static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t filesize,
                   unsigned char *argumentdata, int argumentSize);
static int run_elf_from_file(const char *filename, const char *funcname, const char *args);

/*
 * Beacon API – capture output into global buffer.
 */
static void BeaconPrintf(int type, const char *fmt, ...) {
    (void)type;
    if (g_output_len >= sizeof(g_beacon_output) - 1) return;
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(g_beacon_output + g_output_len,
                            sizeof(g_beacon_output) - g_output_len - 1,
                            fmt, args);
    va_end(args);
    if (written > 0) {
        size_t avail = sizeof(g_beacon_output) - g_output_len - 1;
        if ((size_t)written > avail) written = (int)avail;
        g_output_len += (size_t)written;
    }
}

static void BeaconOutput(int type, const char *data, int len) {
    (void)type;
    if (len <= 0 || data == NULL) return;
    if (g_output_len + (size_t)len >= sizeof(g_beacon_output)) {
        len = (int)(sizeof(g_beacon_output) - g_output_len - 1);
    }
    if (len > 0) {
        memcpy(g_beacon_output + g_output_len, data, (size_t)len);
        g_output_len += (size_t)len;
        g_beacon_output[g_output_len] = '\0';
    }
}

/*
 * Trampoline creation and management.
 */
static void *create_trampoline(void *target) {
    if (target == NULL) return NULL;
    size_t code_size = 12;  /* movabs rax, imm64; jmp rax */
    void *code = mmap(NULL, code_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (code == MAP_FAILED) return NULL;

    unsigned char trampoline_code[] = {
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xE0
    };
    *(uint64_t *)(trampoline_code + 2) = (uint64_t)target;
    memcpy(code, trampoline_code, code_size);

    if (g_trampolines_count >= g_trampolines_capacity) {
        size_t new_cap = (g_trampolines_capacity == 0) ? 4 :
                          g_trampolines_capacity * 2;
        Trampoline *tmp = realloc(g_trampolines, new_cap * sizeof(Trampoline));
        if (tmp == NULL) {
            munmap(code, code_size);
            return NULL;
        }
        g_trampolines = tmp;
        g_trampolines_capacity = new_cap;
    }
    g_trampolines[g_trampolines_count].addr = code;
    g_trampolines[g_trampolines_count].size = code_size;
    g_trampolines_count++;

    return code;
}

static void cleanup_trampolines(void) {
    for (size_t i = 0; i < g_trampolines_count; i++) {
        munmap(g_trampolines[i].addr, g_trampolines[i].size);
    }
    free(g_trampolines);
    g_trampolines = NULL;
    g_trampolines_count = 0;
    g_trampolines_capacity = 0;

    free(g_trampoline_cache);
    g_trampoline_cache = NULL;
    g_cache_count = 0;
    g_cache_capacity = 0;
}

static void *get_or_create_trampoline(void *target) {
    if (target == NULL) return NULL;

    for (size_t i = 0; i < g_cache_count; i++) {
        if (g_trampoline_cache[i].original == target) {
            return g_trampoline_cache[i].trampoline;
        }
    }

    void *tramp = create_trampoline(target);
    if (tramp == NULL) return NULL;

    if (g_cache_count >= g_cache_capacity) {
        size_t new_cap = (g_cache_capacity == 0) ?
                         g_config.trampoline_cache_initial :
                         g_cache_capacity * g_config.trampoline_cache_grow;
        TrampolineCache *tmp = realloc(g_trampoline_cache,
                                       new_cap * sizeof(TrampolineCache));
        if (tmp == NULL) {
            munmap(tramp, 12);
            return NULL;
        }
        g_trampoline_cache = tmp;
        g_cache_capacity = new_cap;
    }
    g_trampoline_cache[g_cache_count].original = target;
    g_trampoline_cache[g_cache_count].trampoline = tramp;
    g_cache_count++;

    return tramp;
}

/*
 * Page alignment utility.
 */
static size_t page_align(size_t size) {
    if (g_config.page_size == 0) {
        long ps = sysconf(_SC_PAGESIZE);
        if (ps <= 0) ps = 4096;
        ((struct hotswap_config *)&g_config)->page_size = (size_t)ps;
    }
    return (size + g_config.page_size - 1) & ~(g_config.page_size - 1);
}

/*
 * Isolated call to BOF function with correct stack alignment.
 */
static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t arglen) {
    __asm__ volatile (
        "push %%rbp\n\t"
        "mov %%rsp, %%rbp\n\t"
        "push %%rbx\n\t"
        "push %%r12\n\t"
        "push %%r13\n\t"
        "push %%r14\n\t"
        "push %%r15\n\t"
        "sub $8, %%rsp\n\t"
        "mov %0, %%rdi\n\t"
        "mov %1, %%rsi\n\t"
        "xor %%eax, %%eax\n\t"
        "call *%2\n\t"
        "add $8, %%rsp\n\t"
        "pop %%r15\n\t"
        "pop %%r14\n\t"
        "pop %%r13\n\t"
        "pop %%r12\n\t"
        "pop %%rbx\n\t"
        "pop %%rbp\n\t"
        :
        : "r"(args), "r"(arglen), "r"(func)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory", "cc"
    );
}

/*
 * Resolve all external symbols needed by the BOF.
 */
static int resolve_external_symbols(void) {
    for (int i = 0; g_external_symbols[i].name; i++) {
        if (*g_external_symbols[i].ptr == NULL) {
            *g_external_symbols[i].ptr = dlsym(RTLD_DEFAULT,
                                               g_external_symbols[i].name);
            if (*g_external_symbols[i].ptr == NULL) {
                fprintf(stderr, "[ERROR] dlsym failed for %s: %s\n",
                        g_external_symbols[i].name, dlerror());
                return -1;
            }
        }
    }
    return 0;
}

/*
 * Core ELF loader: maps sections, resolves relocations, executes entry point.
 */
static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t filesize,
                   unsigned char *argumentdata, int argumentSize) {
    if (elf_data == NULL || filesize < sizeof(Elf64_Ehdr)) {
        return -1;
    }

    /* Initialize function pointers if not already set. */
    if (g_BeaconPrintf_ptr == NULL) g_BeaconPrintf_ptr = (void *)BeaconPrintf;
    if (g_BeaconOutput_ptr == NULL) g_BeaconOutput_ptr = (void *)BeaconOutput;
    if (g_mmap_ptr == NULL) g_mmap_ptr = (void *)mmap;
    if (g_munmap_ptr == NULL) g_munmap_ptr = (void *)munmap;
    if (g_mprotect_ptr == NULL) g_mprotect_ptr = (void *)mprotect;
    if (g_printf_ptr == NULL) g_printf_ptr = (void *)printf;
    if (g_memcpy_ptr == NULL) g_memcpy_ptr = (void *)memcpy;
    if (g_memset_ptr == NULL) g_memset_ptr = (void *)memset;
    if (g_strlen_ptr == NULL) g_strlen_ptr = (void *)strlen;
    if (g_write_ptr == NULL) g_write_ptr = (void *)write;
    if (g_exit_ptr == NULL) g_exit_ptr = (void *)exit;
    if (g_dlsym_ptr == NULL) g_dlsym_ptr = (void *)dlsym;
    if (g_dlerror_ptr == NULL) g_dlerror_ptr = (void *)dlerror;
    if (g_dlopen_ptr == NULL) g_dlopen_ptr = (void *)dlopen;
    if (g_dlclose_ptr == NULL) g_dlclose_ptr = (void *)dlclose;

    g_output_len = 0;
    g_beacon_output[0] = '\0';

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_data;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        return -1;
    }
    if (ehdr->e_machine != EM_X86_64) {
        return -1;
    }
    if (ehdr->e_shoff == 0 || ehdr->e_shnum == 0) {
        return -1;
    }

    Elf64_Shdr *shdr = (Elf64_Shdr *)(elf_data + ehdr->e_shoff);
    char *strtab = NULL;
    Elf64_Sym *symtab = NULL;
    int sym_table_count = 0;

    /* Locate symbol table and string table. */
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            symtab = (Elf64_Sym *)(elf_data + shdr[i].sh_offset);
            if (shdr[i].sh_link < ehdr->e_shnum) {
                strtab = (char *)(elf_data + shdr[shdr[i].sh_link].sh_offset);
            }
            sym_table_count = (int)(shdr[i].sh_size / sizeof(Elf64_Sym));
            break;
        }
    }
    if (symtab == NULL || strtab == NULL) {
        return -1;
    }

    /* Pre‑resolve all undefined symbols (phase 1). */
    for (int i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr *sh = &shdr[i];
        if (sh->sh_type != SHT_RELA) continue;
        Elf64_Rela *rela = (Elf64_Rela *)(elf_data + sh->sh_offset);
        int num_rela = (int)(sh->sh_size / sizeof(Elf64_Rela));
        for (int j = 0; j < num_rela; j++) {
            int sym_idx = ELF64_R_SYM(rela[j].r_info);
            if (sym_idx == 0 || sym_idx >= sym_table_count) continue;
            Elf64_Sym *sym = &symtab[sym_idx];
            if (sym->st_shndx != SHN_UNDEF) continue;
            char *sym_name = strtab + sym->st_name;
            if (sym_name == NULL || sym_name[0] == '\0') continue;

            void *resolved = NULL;
            for (int k = 0; g_external_symbols[k].name; k++) {
                if (strcmp(sym_name, g_external_symbols[k].name) == 0) {
                    if (*g_external_symbols[k].ptr == NULL) {
                        *g_external_symbols[k].ptr = dlsym(RTLD_DEFAULT, sym_name);
                    }
                    resolved = *g_external_symbols[k].ptr;
                    break;
                }
            }
            if (resolved == NULL) {
                resolved = dlsym(RTLD_DEFAULT, sym_name);
            }
            if (resolved == NULL) {
                fprintf(stderr, "[WARNING] Could not pre‑resolve '%s'\n", sym_name);
            }
        }
    }

    /* Allocate section memory (RW). */
    void **sections = calloc((size_t)ehdr->e_shnum, sizeof(void *));
    size_t *aligned_sizes = calloc((size_t)ehdr->e_shnum, sizeof(size_t));
    if (sections == NULL || aligned_sizes == NULL) {
        free(sections);
        free(aligned_sizes);
        return -1;
    }

    for (int i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr *sh = &shdr[i];
        if ((sh->sh_flags & SHF_ALLOC) && sh->sh_size > 0) {
            if (sh->sh_type == SHT_PROGBITS &&
                sh->sh_offset + sh->sh_size > filesize) {
                continue;
            }
            size_t aligned = page_align(sh->sh_size);
            void *addr = mmap(NULL, aligned, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (addr == MAP_FAILED) {
                continue;
            }
            sections[i] = addr;
            aligned_sizes[i] = aligned;

            if (sh->sh_type == SHT_PROGBITS) {
                memcpy(addr, elf_data + sh->sh_offset, sh->sh_size);
            } else if (sh->sh_type == SHT_NOBITS) {
                memset(addr, 0, aligned);
            }
        }
    }

    /* Apply relocations (phase 3). */
    for (int i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr *sh = &shdr[i];
        if (sh->sh_type != SHT_RELA) continue;
        int target_sec = (int)sh->sh_info;
        if (target_sec >= ehdr->e_shnum || sections[target_sec] == NULL) continue;
        if (!(shdr[target_sec].sh_flags & SHF_ALLOC)) continue;

        Elf64_Rela *rela = (Elf64_Rela *)(elf_data + sh->sh_offset);
        int num_rela = (int)(sh->sh_size / sizeof(Elf64_Rela));
        void *target_addr = sections[target_sec];

        for (int j = 0; j < num_rela; j++) {
            Elf64_Rela *r = &rela[j];
            void *loc = (char *)target_addr + r->r_offset;
            int sym_idx = ELF64_R_SYM(r->r_info);
            if (sym_idx == 0 || sym_idx >= sym_table_count) continue;

            Elf64_Sym *sym = &symtab[sym_idx];
            char *sym_name = strtab + sym->st_name;
            void *symbol_addr = NULL;

            if (sym->st_shndx != SHN_UNDEF && sym->st_shndx < ehdr->e_shnum &&
                sections[sym->st_shndx] != NULL) {
                symbol_addr = sections[sym->st_shndx] + sym->st_value;
            } else {
                for (int k = 0; g_external_symbols[k].name; k++) {
                    if (strcmp(sym_name, g_external_symbols[k].name) == 0) {
                        symbol_addr = *g_external_symbols[k].ptr;
                        break;
                    }
                }
                if (symbol_addr == NULL) {
                    symbol_addr = dlsym(RTLD_DEFAULT, sym_name);
                }
                if (symbol_addr == NULL) {
                    fprintf(stderr, "[ERROR] Cannot resolve '%s'\n", sym_name);
                    goto cleanup;
                }
            }

            switch (ELF64_R_TYPE(r->r_info)) {
                case R_X86_64_64:
                    *(uint64_t *)loc = (uint64_t)((uintptr_t)symbol_addr + r->r_addend);
                    break;
                case R_X86_64_32:
                    {
                        uintptr_t val = (uintptr_t)symbol_addr + r->r_addend;
                        if (val > UINT32_MAX) goto cleanup;
                        *(uint32_t *)loc = (uint32_t)val;
                    }
                    break;
                case R_X86_64_32S:
                    {
                        intptr_t val = (intptr_t)symbol_addr + r->r_addend;
                        if (val < INT32_MIN || val > INT32_MAX) goto cleanup;
                        *(int32_t *)loc = (int32_t)val;
                    }
                    break;
                case R_X86_64_PC32:
                case R_X86_64_PLT32:
                    {
                        int64_t offset = (int64_t)symbol_addr + r->r_addend -
                                         (int64_t)loc;
                        if (offset < INT32_MIN || offset > INT32_MAX) {
                            void *tramp = get_or_create_trampoline(symbol_addr);
                            if (tramp == NULL) goto cleanup;
                            offset = (int64_t)tramp + r->r_addend -
                                     (int64_t)loc;
                            if (offset < INT32_MIN || offset > INT32_MAX)
                                goto cleanup;
                        }
                        *(int32_t *)loc = (int32_t)offset;
                    }
                    break;
                default:
                    fprintf(stderr, "[ERROR] Unsupported relocation %ld for %s\n",
                            ELF64_R_TYPE(r->r_info), sym_name);
                    goto cleanup;
            }
        }
    }

    /* Locate entry point. */
    void (*entry)(char *, int) = NULL;
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            int sym_count = (int)(shdr[i].sh_size / sizeof(Elf64_Sym));
            for (int j = 0; j < sym_count; j++) {
                Elf64_Sym *sym = &symtab[j];
                if (sym->st_name == 0) continue;
                char *name = strtab + sym->st_name;
                if (strcmp(name, functionname) == 0 &&
                    sym->st_shndx != SHN_UNDEF &&
                    sym->st_shndx < ehdr->e_shnum &&
                    sections[sym->st_shndx] != NULL) {
                    entry = (void (*)(char *, int))(sections[sym->st_shndx] +
                                                    sym->st_value);
                    break;
                }
            }
            if (entry) break;
        }
    }
    if (entry == NULL) {
        goto cleanup;
    }

    /* Make sections executable (W^X). */
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (sections[i] != NULL) {
            if (mprotect(sections[i], aligned_sizes[i],
                         PROT_READ | PROT_EXEC) != 0) {
                goto cleanup;
            }
        }
    }

    /* Execute BOF. */
    call_bof_isolated(entry, (char *)argumentdata, (uintptr_t)argumentSize);

cleanup:
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (sections[i] != NULL) {
            munmap(sections[i], aligned_sizes[i]);
        }
    }
    free(sections);
    free(aligned_sizes);
    cleanup_trampolines();
    return (entry != NULL) ? 0 : -1;
}

/*
 * Convenience wrapper: load ELF from file and run.
 */
static int run_elf_from_file(const char *filename, const char *funcname,
                             const char *args) {
    if (filename == NULL) return -1;

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("fstat");
        close(fd);
        return -1;
    }
    if (st.st_size <= 0 || (uint64_t)st.st_size > SIZE_MAX) {
        close(fd);
        return -1;
    }

    unsigned char *elf_data = malloc((size_t)st.st_size);
    if (elf_data == NULL) {
        close(fd);
        return -1;
    }

    ssize_t n = read(fd, elf_data, (size_t)st.st_size);
    close(fd);
    if (n != (ssize_t)st.st_size) {
        free(elf_data);
        return -1;
    }

    int ret = run_elf(funcname, elf_data, (uint32_t)st.st_size,
                      (unsigned char *)args, (int)strlen(args));

    if (g_output_len > 0) {
        fwrite(g_beacon_output, 1, g_output_len, stdout);
        fputc('\n', stdout);
    }

    free(elf_data);
    return ret;
}

/*
 * ============================================================================
 *   ORIGINAL mini_gcc COMPILER CODE (slightly adapted for integration)
 * ============================================================================
 */

#define MAX_TOKEN_LEN   64
#define MAX_SYMBOLS     2048
#define MAX_IDENT_LEN   32
#define MAX_SOURCE_SIZE 1048576
#define STACK_ALIGN     16

enum {
    T_NUM = 256,
    T_ID,
    T_IF,
    T_ELSE,
    T_WHILE,
    T_RETURN,
    T_INT,
    T_CHAR,
    T_VOID,
    T_ENUM,
    T_STATIC,
    T_TYPEDEF,
    T_STRUCT,
    T_CONST,
    T_FOR,
    T_INC,
    T_DEC,
    T_ARROW,
    T_LE,
    T_GE,
    T_EQ,
    T_NE,
    T_AND,
    T_OR,
    T_SWITCH,
    T_CASE,
    T_DEFAULT,
    T_BREAK,
    T_CONTINUE,
    T_STRING,
    T_ADD_ASSIGN,
    T_SUB_ASSIGN,
    T_GOTO,
    T_EOF
};

static char *input_ptr;
static char *source_start;
static char token[MAX_TOKEN_LEN];
static int tok;
static int line = 1;
static FILE *output;

typedef struct {
    char name[MAX_IDENT_LEN];
    int offset;
    int is_global;
    int size;
    int pointed;
    int is_const;
    int const_value;
    int is_array;
    int elem_size;
    int elem_size2;
} Symbol;

static Symbol symbols[MAX_SYMBOLS];
static int symbol_count = 0;
static int stack_size = 0;
static int label_counter = 0;
static int function_has_return = 0;
static int emit_enabled = 1;
static int max_func_stack = 0;
static int assign_size = 8;
static int expr_pointed = 0;
static int current_elem_size = 0;
static int current_elem_size2 = 0;
static int no_postfix_deref = 0;

#define MAX_CASES_PER_SWITCH 256
static int switch_case_values[MAX_CASES_PER_SWITCH];
static int switch_case_labels[MAX_CASES_PER_SWITCH];
static int switch_case_count = 0;
static int switch_has_default = 0;
static int switch_default_label = 0;
static int break_target = 0;
static int break_target_valid = 0;
static int continue_target = 0;
static int continue_target_valid = 0;

static int str_label_counter = 0;
static int peek_mode = 0;
#define MAX_STRINGS 2048
static char *string_pool[MAX_STRINGS];
static int string_count = 0;

#define MAX_STRUCT_MEMBERS 256
static int struct_total_size = 0;
static char struct_member_names[MAX_STRUCT_MEMBERS][MAX_IDENT_LEN];
static int struct_member_offsets[MAX_STRUCT_MEMBERS];
static int struct_member_sizes[MAX_STRUCT_MEMBERS];
static int struct_member_elem_sizes[MAX_STRUCT_MEMBERS];
static int struct_member_count = 0;

static void error(const char *msg);

#define MAX_MACROS 256
static int macro_count = 0;
typedef struct {
    char name[MAX_IDENT_LEN];
    int value;
} Macro;
static Macro macros[MAX_MACROS];

typedef struct {
    int assign_size;
    int expr_pointed;
    int current_elem_size;
    int current_elem_size2;
    int no_postfix_deref;
    int symbol_count;
    int stack_size;
    int function_has_return;
    int emit_enabled;
    int max_func_stack;
    int switch_case_count;
    int switch_has_default;
    int switch_default_label;
    int break_target;
    int break_target_valid;
    int continue_target;
    int continue_target_valid;
    int peek_mode;
    int struct_total_size;
    int struct_member_count;
    int macro_count;
} ParserState;

static void save_parser_state(ParserState *state);
static void restore_parser_state(ParserState *state);
static int find_macro(const char *name);
static void add_macro(const char *name, int value);
static void *safe_malloc(size_t size);
static int my_isspace(int c);
static int my_isalpha(int c);
static int my_isdigit(int c);
static int my_isalnum(int c);
static void next_token(void);
static void match(int expected);
static void emit(const char *s);
static void emit_i(const char *fmt, int v);
static void emit_s(const char *fmt, const char *s);
static void emit_is(const char *fmt, int v, const char *s);
static void emit_si(const char *fmt, const char *s, int v);
static void emit_label(int label);
static int find_symbol(const char *name);
static void add_symbol(const char *name, int is_global, int size, int pointed, int is_array, int elem_size);
static void statement(void);
static void lvalue_address(void);
static void handle_postfix(int is_lvalue);
static void assignment_expr(void);
static void parse_enum(void);
static void skip_struct(void);
static void skip_typedef(void);
static const char *arg_reg(int i);
static const char *libc_global_name(int i);
static void unary(void);
static void unary_expr(void);
static void multiplicative_expr(void);
static void additive_expr(void);
static void relational_expr(void);
static void equality_expr(void);
static void bitwise_and_expr(void);
static void bitwise_xor_expr(void);
static void bitwise_or_expr(void);
static void logical_and_expr(void);
static void logical_or_expr(void);
static void conditional_expr(void);
static void parse_function(const char *name, int ret_type);
static void parse_program(void);
static void emit_string_pool(void);

/*
 * Error handling.
 */
static void error(const char *msg) {
    fprintf(stderr, "Error at line %d, token '%s': %s\n", line, token, msg);
    exit(EXIT_FAILURE);
}

static void *safe_malloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

static int my_isspace(int c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
            c == '\f' || c == '\v');
}

static int my_isalpha(int c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static int my_isdigit(int c) {
    return (c >= '0' && c <= '9');
}

static int my_isalnum(int c) {
    return (my_isalpha(c) || my_isdigit(c));
}

static int find_macro(const char *name) {
    for (int i = 0; i < macro_count; i++) {
        if (strcmp(macros[i].name, name) == 0)
            return i;
    }
    return -1;
}

static void add_macro(const char *name, int value) {
    if (macro_count >= MAX_MACROS)
        error("too many macros");
    size_t nlen = strlen(name);
    if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
    memcpy(macros[macro_count].name, name, nlen);
    macros[macro_count].name[nlen] = '\0';
    macros[macro_count].value = value;
    macro_count++;
}

static void save_parser_state(ParserState *state) {
    state->assign_size = assign_size;
    state->expr_pointed = expr_pointed;
    state->current_elem_size = current_elem_size;
    state->current_elem_size2 = current_elem_size2;
    state->no_postfix_deref = no_postfix_deref;
    state->symbol_count = symbol_count;
    state->stack_size = stack_size;
    state->function_has_return = function_has_return;
    state->emit_enabled = emit_enabled;
    state->max_func_stack = max_func_stack;
    state->switch_case_count = switch_case_count;
    state->switch_has_default = switch_has_default;
    state->switch_default_label = switch_default_label;
    state->break_target = break_target;
    state->break_target_valid = break_target_valid;
    state->continue_target = continue_target;
    state->continue_target_valid = continue_target_valid;
    state->peek_mode = peek_mode;
    state->struct_total_size = struct_total_size;
    state->struct_member_count = struct_member_count;
    state->macro_count = macro_count;
}

static void restore_parser_state(ParserState *state) {
    assign_size = state->assign_size;
    expr_pointed = state->expr_pointed;
    current_elem_size = state->current_elem_size;
    current_elem_size2 = state->current_elem_size2;
    no_postfix_deref = state->no_postfix_deref;
    symbol_count = state->symbol_count;
    stack_size = state->stack_size;
    function_has_return = state->function_has_return;
    emit_enabled = state->emit_enabled;
    max_func_stack = state->max_func_stack;
    switch_case_count = state->switch_case_count;
    switch_has_default = state->switch_has_default;
    switch_default_label = state->switch_default_label;
    break_target = state->break_target;
    break_target_valid = state->break_target_valid;
    continue_target = state->continue_target;
    continue_target_valid = state->continue_target_valid;
    peek_mode = state->peek_mode;
    struct_total_size = state->struct_total_size;
    struct_member_count = state->struct_member_count;
    macro_count = state->macro_count;
}

static void next_token(void) {
    int c;
restart:
    c = *input_ptr;
    while (my_isspace(c)) {
        if (c == '\n') line++;
        input_ptr++;
        c = *input_ptr;
    }

    if (c == '\0') {
        tok = T_EOF;
        return;
    }

    if (c == '/' && input_ptr[1] == '*') {
        input_ptr += 2;
        while (*input_ptr) {
            if (*input_ptr == '*' && input_ptr[1] == '/') {
                input_ptr += 2;
                break;
            }
            if (*input_ptr == '\n') line++;
            input_ptr++;
        }
        goto restart;
    }
    if (c == '/' && input_ptr[1] == '/') {
        input_ptr += 2;
        while (*input_ptr && *input_ptr != '\n') input_ptr++;
        goto restart;
    }
    if (c == '#') {
        input_ptr++;
        while (my_isspace(*input_ptr)) {
            if (*input_ptr == '\n') line++;
            input_ptr++;
        }
        if (strncmp(input_ptr, "define", 6) == 0) {
            input_ptr += 6;
            while (my_isspace(*input_ptr)) input_ptr++;
            char mname[MAX_IDENT_LEN];
            int mlen = 0;
            while ((my_isalnum(*input_ptr) || *input_ptr == '_') &&
                   mlen < MAX_IDENT_LEN - 1) {
                mname[mlen] = *input_ptr;
                mlen++;
                input_ptr++;
            }
            mname[mlen] = '\0';
            while (my_isspace(*input_ptr)) input_ptr++;
            int mval = 0;
            int has_val = 0;
            if (my_isdigit(*input_ptr)) {
                has_val = 1;
                while (my_isdigit(*input_ptr)) {
                    mval = mval * 10 + (*input_ptr - '0');
                    input_ptr++;
                }
            }
            if (has_val || mlen > 0) {
                add_macro(mname, mval);
            }
        }
        while (*input_ptr && *input_ptr != '\n') input_ptr++;
        goto restart;
    }

    if (my_isalpha(c) || c == '_') {
        char *p = token;
        int len = 0;
        while ((my_isalnum(*input_ptr) || *input_ptr == '_') &&
               len < MAX_TOKEN_LEN - 1) {
            *p = *input_ptr;
            p++;
            input_ptr++;
            len++;
        }
        *p = '\0';
        if (strcmp(token, "if") == 0)           tok = T_IF;
        else if (strcmp(token, "else") == 0)    tok = T_ELSE;
        else if (strcmp(token, "while") == 0)   tok = T_WHILE;
        else if (strcmp(token, "return") == 0)  tok = T_RETURN;
        else if (strcmp(token, "int") == 0)     tok = T_INT;
        else if (strcmp(token, "long") == 0)    tok = T_INT;
        else if (strcmp(token, "char") == 0)    tok = T_CHAR;
        else if (strcmp(token, "void") == 0)    tok = T_VOID;
        else if (strcmp(token, "enum") == 0)    tok = T_ENUM;
        else if (strcmp(token, "static") == 0)  tok = T_STATIC;
        else if (strcmp(token, "typedef") == 0) tok = T_TYPEDEF;
        else if (strcmp(token, "struct") == 0)  tok = T_STRUCT;
        else if (strcmp(token, "const") == 0)   tok = T_CONST;
        else if (strcmp(token, "for") == 0)     tok = T_FOR;
        else if (strcmp(token, "switch") == 0)  tok = T_SWITCH;
        else if (strcmp(token, "case") == 0)    tok = T_CASE;
        else if (strcmp(token, "default") == 0) tok = T_DEFAULT;
        else if (strcmp(token, "break") == 0)   tok = T_BREAK;
        else if (strcmp(token, "continue") == 0) tok = T_CONTINUE;
        else if (strcmp(token, "goto") == 0)    tok = T_GOTO;
        else {
            int mi = find_macro(token);
            if (mi >= 0) {
                snprintf(token, MAX_TOKEN_LEN, "%d", macros[mi].value);
                tok = T_NUM;
            } else {
                tok = T_ID;
            }
        }
        return;
    }

    if (my_isdigit(c)) {
        char *p = token;
        int len = 0;
        while (my_isdigit(*input_ptr) && len < MAX_TOKEN_LEN - 1) {
            *p = *input_ptr;
            p++;
            input_ptr++;
            len++;
        }
        *p = '\0';
        tok = T_NUM;
        return;
    }

    if (c == '"') {
        char *p = token;
        int len = 0;
        input_ptr++;
        while (*input_ptr && *input_ptr != '"' && len < MAX_TOKEN_LEN - 2) {
            if (*input_ptr == '\\' && input_ptr[1]) {
                if (input_ptr[1] == 'n') {
                    *p = '\n'; p++; input_ptr += 2; len++;
                } else if (input_ptr[1] == 't') {
                    *p = '\t'; p++; input_ptr += 2; len++;
                } else if (input_ptr[1] == '\\') {
                    *p = '\\'; p++; input_ptr += 2; len++;
                } else if (input_ptr[1] == '"') {
                    *p = '"'; p++; input_ptr += 2; len++;
                } else {
                    *p = *input_ptr; p++; input_ptr++;
                    *p = *input_ptr; p++; input_ptr++;
                    len += 2;
                }
            } else {
                *p = *input_ptr;
                p++;
                input_ptr++;
                len++;
            }
        }
        *p = '\0';
        if (*input_ptr == '"') input_ptr++;
        else error("unterminated string literal");
        tok = T_STRING;
        return;
    }

    if (c == '\'') {
        input_ptr++;
        int ch;
        if (*input_ptr == '\\') {
            input_ptr++;
            switch (*input_ptr) {
                case 'n': ch = '\n'; break;
                case 't': ch = '\t'; break;
                case 'r': ch = '\r'; break;
                case 'f': ch = '\f'; break;
                case 'v': ch = '\v'; break;
                case '0': ch = '\0'; break;
                case '\\': ch = '\\'; break;
                case '\'': ch = '\''; break;
                default: ch = *input_ptr; break;
            }
        } else {
            ch = *input_ptr;
        }
        input_ptr++;
        if (*input_ptr != '\'') error("unterminated char literal");
        input_ptr++;
        snprintf(token, MAX_TOKEN_LEN, "%d", ch);
        tok = T_NUM;
        return;
    }

    if (c == '=' && input_ptr[1] == '=') {
        input_ptr += 2; tok = T_EQ; strcpy(token, "=="); return;
    }
    if (c == '!' && input_ptr[1] == '=') {
        input_ptr += 2; tok = T_NE; strcpy(token, "!="); return;
    }
    if (c == '<' && input_ptr[1] == '=') {
        input_ptr += 2; tok = T_LE; strcpy(token, "<="); return;
    }
    if (c == '>' && input_ptr[1] == '=') {
        input_ptr += 2; tok = T_GE; strcpy(token, ">="); return;
    }
    if (c == '&' && input_ptr[1] == '&') {
        input_ptr += 2; tok = T_AND; strcpy(token, "&&"); return;
    }
    if (c == '|' && input_ptr[1] == '|') {
        input_ptr += 2; tok = T_OR; strcpy(token, "||"); return;
    }
    if (c == '+' && input_ptr[1] == '+') {
        input_ptr += 2; tok = T_INC; strcpy(token, "++"); return;
    }
    if (c == '+' && input_ptr[1] == '=') {
        input_ptr += 2; tok = T_ADD_ASSIGN; strcpy(token, "+="); return;
    }
    if (c == '-' && input_ptr[1] == '=') {
        input_ptr += 2; tok = T_SUB_ASSIGN; strcpy(token, "-="); return;
    }
    if (c == '-' && input_ptr[1] == '-') {
        input_ptr += 2; tok = T_DEC; strcpy(token, "--"); return;
    }
    if (c == '-' && input_ptr[1] == '>') {
        input_ptr += 2; tok = T_ARROW; strcpy(token, "->"); return;
    }

    token[0] = c;
    token[1] = '\0';
    tok = c;
    input_ptr++;
}

static void match(int expected) {
    if (tok == expected) next_token();
    else error("unexpected token");
}

static void emit(const char *s) {
    if (!emit_enabled || peek_mode) return;
    while (*s) {
        if (*s == '%' && s[1] == '%') {
            fputc('%', output);
            s += 2;
        } else {
            fputc(*s, output);
            s++;
        }
    }
    fputc('\n', output);
}

static void emit_i(const char *fmt, int v) {
    if (!emit_enabled || peek_mode) return;
    fprintf(output, fmt, v);
    fputc('\n', output);
}

static void emit_s(const char *fmt, const char *s) {
    if (!emit_enabled || peek_mode) return;
    fprintf(output, fmt, s);
    fputc('\n', output);
}

static void emit_is(const char *fmt, int v, const char *s) {
    if (!emit_enabled || peek_mode) return;
    fprintf(output, fmt, v, s);
    fputc('\n', output);
}

static void emit_si(const char *fmt, const char *s, int v) {
    if (!emit_enabled || peek_mode) return;
    fprintf(output, fmt, s, v);
    fputc('\n', output);
}

static void emit_label(int label) {
    if (emit_enabled && !peek_mode)
        fprintf(output, ".L%d:\n", label);
}

static int find_symbol(const char *name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbols[i].name, name) == 0)
            return i;
    }
    return -1;
}

static void add_symbol(const char *name, int is_global, int size, int pointed,
                       int is_array, int elem_size) {
    if (symbol_count >= MAX_SYMBOLS)
        error("too many symbols");
    Symbol *s = &symbols[symbol_count];
    size_t nlen = strlen(name);
    if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
    memcpy(s->name, name, nlen);
    s->name[nlen] = '\0';
    s->is_global = is_global;
    s->size = size;
    s->pointed = pointed;
    s->is_const = 0;
    s->const_value = 0;
    s->is_array = is_array;
    s->elem_size = elem_size;
    s->elem_size2 = 0;
    if (is_global) {
        s->offset = 0;
        emit("    .bss");
        emit_s("    .globl %s", name);
        emit_s("%s:", name);
        if (size > 0)
            emit_i("    .space %d", size);
        emit("    .text");
    } else {
        stack_size = (stack_size + size + STACK_ALIGN - 1) & ~(STACK_ALIGN - 1);
        s->offset = -stack_size;
        if (stack_size > max_func_stack)
            max_func_stack = stack_size;
    }
    symbol_count++;
}

static const char *arg_reg(int i) {
    if (i == 0) return "%rdi";
    if (i == 1) return "%rsi";
    if (i == 2) return "%rdx";
    if (i == 3) return "%rcx";
    if (i == 4) return "%r8";
    return "%r9";
}

static const char *libc_global_name(int i) {
    if (i == 0) return "stderr";
    if (i == 1) return "stdin";
    if (i == 2) return "stdout";
    if (i == 3) return "optarg";
    if (i == 4) return "optind";
    if (i == 5) return "errno";
    if (i == 6) return "size_t";
    if (i == 7) return "va_list";
    if (i == 8) return "FILE";
    return NULL;
}

static void unary(void) {
    if (tok == T_NUM) {
        emit_s("    movq $%s, %%rax", token);
        expr_pointed = 0;
        next_token();
    } else if (tok == T_ID) {
        char id_name[MAX_IDENT_LEN];
        strcpy(id_name, token);
        next_token();
        if (tok == '(') {
            next_token();
            int argc = 0;
            if (tok != ')') {
                while (1) {
                    assignment_expr();
                    emit("    pushq %%rax");
                    argc++;
                    if (tok == ')') break;
                    match(',');
                }
            }
            match(')');
            for (int i = 0; i < argc && i < 6; i++)
                emit_is("    movq %d(%%rsp), %s", (argc - i - 1) * 8, arg_reg(i));
            if (argc > 6)
                error("too many function arguments (max 6)");
            if (argc > 0)
                emit_i("    addq $%d, %%rsp", argc * 8);
            emit("    pushq %%r12");
            emit("    movq %%rsp, %%r12");
            emit("    andq $-16, %%rsp");
            emit("    xorl %%eax, %%eax");
            emit_s("    call %s", id_name);
            emit("    movq %%r12, %%rsp");
            emit("    popq %%r12");
            expr_pointed = 0;
        } else {
            int idx = find_symbol(id_name);
            if (idx < 0) error("undefined variable");
            Symbol *s = &symbols[idx];
            if (s->is_const) {
                emit_i("    movq $%d, %%rax", s->const_value);
                expr_pointed = 0;
            } else if (s->is_array || (s->pointed && s->size > 8)) {
                current_elem_size = s->is_array ? s->elem_size : 8;
                current_elem_size2 = s->is_array ? s->elem_size2 : 0;
                if (s->is_global)
                    emit_s("    leaq %s(%%rip), %%rax", id_name);
                else
                    emit_i("    leaq %d(%%rbp), %%rax", s->offset);
                expr_pointed = s->size > 0 && s->size <= 8 ? 0 :
                              (s->pointed ? s->pointed : T_INT);
            } else {
                expr_pointed = s->pointed;
                current_elem_size2 = 0;
                if (s->pointed) current_elem_size = (s->pointed == T_CHAR) ? 1 : 8;
                else current_elem_size = 0;
                if (s->size == 1) {
                    if (s->is_global)
                        emit_s("    movsbq %s(%%rip), %%rax", id_name);
                    else
                        emit_i("    movsbq %d(%%rbp), %%rax", s->offset);
                } else {
                    if (s->is_global)
                        emit_s("    movq %s(%%rip), %%rax", id_name);
                    else
                        emit_i("    movq %d(%%rbp), %%rax", s->offset);
                }
            }
        }
    } else if (tok == '(') {
        char *cast_save = input_ptr;
        int cast_line = line;
        next_token();
        int is_cast = 0;
        if (tok == T_ID) {
            int ti = find_symbol(token);
            if (ti >= 0 && symbols[ti].is_const) {
                next_token();
                while (tok == '*') next_token();
                if (tok == ')') is_cast = 1;
            }
        } else if (tok == T_INT || tok == T_CHAR) {
            int saved = tok;
            next_token();
            while (tok == '*') next_token();
            if (tok == ')') is_cast = 1;
            if (!is_cast) tok = saved;
        }
        if (is_cast) {
            next_token();
            unary();
        } else {
            input_ptr = cast_save;
            line = cast_line;
            tok = '(';
            next_token();
            assignment_expr();
            match(')');
        }
    } else if (tok == '*') {
        next_token();
        unary();
        if (expr_pointed == T_CHAR)
            emit("    movsbq (%%rax), %%rax");
        else
            emit("    movq (%%rax), %%rax");
    } else if (tok == '&') {
        next_token();
        if (tok != T_ID) error("expected identifier after '&'");
        int idx = find_symbol(token);
        if (idx < 0) error("undefined variable");
        Symbol *s = &symbols[idx];
        expr_pointed = (s->size == 1) ? T_CHAR : T_INT;
        current_elem_size = s->is_array ? s->elem_size : 0;
        current_elem_size2 = s->is_array ? s->elem_size2 : 0;
        no_postfix_deref = 1;
        if (s->is_global)
            emit_s("    leaq %s(%%rip), %%rax", token);
        else
            emit_i("    leaq %d(%%rbp), %%rax", s->offset);
        next_token();
    } else if (tok == T_STRING) {
        int lbl = str_label_counter++;
        if (string_count < MAX_STRINGS) {
            string_pool[string_count] = malloc(strlen(token) + 1);
            strcpy(string_pool[string_count], token);
            string_count++;
        }
        emit_i("    leaq .Lstr%d(%%rip), %%rax", lbl);
        expr_pointed = T_CHAR;
        next_token();
    } else if (tok == '-') {
        next_token();
        unary();
        handle_postfix(0);
        emit("    negq %%rax");
    } else if (tok == '!') {
        next_token();
        unary();
        handle_postfix(0);
        emit("    testq %%rax, %%rax");
        emit("    sete %%al");
        emit("    movzbq %%al, %%rax");
    } else if (tok == '~') {
        next_token();
        unary();
        handle_postfix(0);
        emit("    notq %%rax");
    } else {
        error("invalid primary expression");
    }
}

static void lvalue_address(void) {
    no_postfix_deref = 0;
    if (tok == T_ID) {
        int idx = find_symbol(token);
        if (idx < 0) error("undefined variable");
        Symbol *s = &symbols[idx];
        assign_size = s->size;
        if (s->is_array || (s->pointed && s->size > 8)) {
            current_elem_size = s->is_array ? s->elem_size : 8;
            current_elem_size2 = s->is_array ? s->elem_size2 : 0;
            expr_pointed = s->pointed ? s->pointed : T_INT;
        } else {
            current_elem_size2 = 0;
            expr_pointed = s->pointed;
            if (s->pointed) current_elem_size = (s->pointed == T_CHAR) ? 1 : 8;
            else current_elem_size = 0;
        }
        int need_ptr_value = (s->pointed && s->size == 8);
        next_token();
        if (need_ptr_value && (tok == '[' || tok == '.' || tok == T_ARROW || tok == '('))
            need_ptr_value = 1;
        else if (need_ptr_value)
            need_ptr_value = 0;
        if (need_ptr_value) {
            if (s->is_global)
                emit_s("    movq %s(%%rip), %%rax", s->name);
            else
                emit_i("    movq %d(%%rbp), %%rax", s->offset);
        } else {
            if (s->is_global)
                emit_s("    leaq %s(%%rip), %%rax", s->name);
            else
                emit_i("    leaq %d(%%rbp), %%rax", s->offset);
        }
        handle_postfix(1);
    } else if (tok == '*') {
        next_token();
        unary();
        handle_postfix(1);
        if (assign_size == 0) assign_size = (expr_pointed == T_CHAR) ? 1 : 8;
    } else {
        error("lvalue required");
    }
}

static void handle_postfix(int is_lvalue) {
    while (tok == '[' || tok == '.' || tok == T_ARROW) {
        if (tok == '[') {
            next_token();
            emit("    pushq %%rax");
            int saved_ep = expr_pointed;
            int saved_ces = current_elem_size;
            int saved_ces2 = current_elem_size2;
            int saved_as = assign_size;
            int saved_npd = no_postfix_deref;

            assignment_expr();

            expr_pointed = saved_ep;
            current_elem_size = saved_ces;
            current_elem_size2 = saved_ces2;
            assign_size = saved_as;
            no_postfix_deref = saved_npd;

            emit("    popq %%rcx");
            int elem_size = current_elem_size;
            if (elem_size == 0) elem_size = (expr_pointed == T_CHAR) ? 1 : 8;
            if (elem_size > 1) emit_i("    imulq $%d, %%rax", elem_size);
            emit("    addq %%rcx, %%rax");
            current_elem_size = elem_size;
            assign_size = elem_size;

            if (current_elem_size2 > 0) {
                current_elem_size = current_elem_size2;
                current_elem_size2 = 0;
                assign_size = current_elem_size;
            } else if (!is_lvalue && !no_postfix_deref && elem_size <= 8) {
                if (elem_size == 1)
                    emit("    movsbq (%%rax), %%rax");
                else
                    emit("    movq (%%rax), %%rax");
                expr_pointed = 0;
            }
            match(']');
        } else if (tok == '.') {
            next_token();
            int off = 0, msize = 8, mesize = 8;
            for (int i = 0; i < struct_member_count; i++) {
                if (strcmp(token, struct_member_names[i]) == 0) {
                    off = struct_member_offsets[i];
                    msize = struct_member_sizes[i];
                    mesize = struct_member_elem_sizes[i];
                    break;
                }
            }
            next_token();
            if (off > 0) emit_i("    addq $%d, %%rax", off);
            assign_size = msize;
            current_elem_size = mesize;
            current_elem_size2 = 0;
            if (is_lvalue) {
                expr_pointed = (msize > 8) ? T_INT : 0;
            } else {
                if (msize > 8) {
                    expr_pointed = (msize > 0) ? T_INT : 0;
                } else {
                    if (msize == 1)
                        emit("    movsbq (%%rax), %%rax");
                    else
                        emit("    movq (%%rax), %%rax");
                    expr_pointed = 0;
                }
            }
        } else if (tok == T_ARROW) {
            next_token();
            int off = 0, msize = 8, mesize = 8;
            for (int i = 0; i < struct_member_count; i++) {
                if (strcmp(token, struct_member_names[i]) == 0) {
                    off = struct_member_offsets[i];
                    msize = struct_member_sizes[i];
                    mesize = struct_member_elem_sizes[i];
                    break;
                }
            }
            next_token();
            if (off > 0) emit_i("    addq $%d, %%rax", off);
            assign_size = msize;
            current_elem_size = mesize;
            current_elem_size2 = 0;
            if (is_lvalue) {
                expr_pointed = (msize > 8) ? T_INT : 0;
            } else {
                if (msize > 8) {
                    expr_pointed = (msize > 0) ? T_INT : 0;
                } else {
                    if (msize == 1)
                        emit("    movsbq (%%rax), %%rax");
                    else
                        emit("    movq (%%rax), %%rax");
                    expr_pointed = 0;
                }
            }
        }
    }
}

static void unary_expr(void) {
    unary();
    handle_postfix(0);
}

static void multiplicative_expr(void) {
    unary_expr();
    while (tok == '*' || tok == '/' || tok == '%') {
        int op = tok;
        next_token();
        emit("    pushq %%rax");
        unary_expr();
        emit("    popq %%rcx");
        if (op == '*') {
            emit("    imulq %%rcx, %%rax");
        } else if (op == '/') {
            emit("    movq %%rax, %%r8");
            emit("    movq %%rcx, %%rax");
            emit("    cqto");
            emit("    idivq %%r8");
        } else {
            emit("    movq %%rax, %%r8");
            emit("    movq %%rcx, %%rax");
            emit("    cqto");
            emit("    idivq %%r8");
            emit("    movq %%rdx, %%rax");
        }
        expr_pointed = 0;
    }
}

static void additive_expr(void) {
    multiplicative_expr();
    while (tok == '+' || tok == '-') {
        int op = tok;
        next_token();
        emit("    pushq %%rax");
        multiplicative_expr();
        emit("    popq %%rcx");
        if (op == '+') {
            emit("    addq %%rcx, %%rax");
        } else {
            emit("    subq %%rax, %%rcx");
            emit("    movq %%rcx, %%rax");
        }
        expr_pointed = 0;
    }
}

static void relational_expr(void) {
    additive_expr();
    while (tok == '<' || tok == T_LE || tok == '>' || tok == T_GE) {
        int op = tok;
        next_token();
        emit("    pushq %%rax");
        additive_expr();
        emit("    popq %%rcx");
        emit("    cmpq %%rax, %%rcx");
        if (op == '<') emit("    setl %%al");
        else if (op == T_LE) emit("    setle %%al");
        else if (op == '>') emit("    setg %%al");
        else emit("    setge %%al");
        emit("    movzbq %%al, %%rax");
        expr_pointed = 0;
    }
}

static void equality_expr(void) {
    relational_expr();
    while (tok == T_EQ || tok == T_NE) {
        int op = tok;
        next_token();
        emit("    pushq %%rax");
        relational_expr();
        emit("    popq %%rcx");
        emit("    cmpq %%rax, %%rcx");
        if (op == T_EQ) emit("    sete %%al");
        else emit("    setne %%al");
        emit("    movzbq %%al, %%rax");
        expr_pointed = 0;
    }
}

static void bitwise_and_expr(void) {
    equality_expr();
    while (tok == '&') {
        next_token();
        emit("    pushq %%rax");
        equality_expr();
        emit("    popq %%rcx");
        emit("    andq %%rcx, %%rax");
        expr_pointed = 0;
    }
}

static void bitwise_xor_expr(void) {
    bitwise_and_expr();
    while (tok == '^') {
        next_token();
        emit("    pushq %%rax");
        bitwise_and_expr();
        emit("    popq %%rcx");
        emit("    xorq %%rcx, %%rax");
        expr_pointed = 0;
    }
}

static void bitwise_or_expr(void) {
    bitwise_xor_expr();
    while (tok == '|') {
        next_token();
        emit("    pushq %%rax");
        bitwise_xor_expr();
        emit("    popq %%rcx");
        emit("    orq %%rcx, %%rax");
        expr_pointed = 0;
    }
}

static void logical_and_expr(void) {
    bitwise_or_expr();
    while (tok == T_AND) {
        next_token();
        int l_false = label_counter++;
        int l_end = label_counter++;
        emit("    testq %%rax, %%rax");
        emit_i("    je .L%d", l_false);
        bitwise_or_expr();
        emit("    testq %%rax, %%rax");
        emit_i("    je .L%d", l_false);
        emit("    movl $1, %%eax");
        emit_i("    jmp .L%d", l_end);
        emit_label(l_false);
        emit("    xorl %%eax, %%eax");
        emit_label(l_end);
        expr_pointed = 0;
    }
}

static void logical_or_expr(void) {
    logical_and_expr();
    while (tok == T_OR) {
        next_token();
        int l_true = label_counter++;
        int l_end = label_counter++;
        emit("    testq %%rax, %%rax");
        emit_i("    jne .L%d", l_true);
        logical_and_expr();
        emit("    testq %%rax, %%rax");
        emit_i("    jne .L%d", l_true);
        emit("    xorl %%eax, %%eax");
        emit_i("    jmp .L%d", l_end);
        emit_label(l_true);
        emit("    movl $1, %%eax");
        emit_label(l_end);
        expr_pointed = 0;
    }
}

static void conditional_expr(void) {
    logical_or_expr();
    if (tok == '?') {
        int l_false = label_counter++;
        int l_end = label_counter++;
        next_token();
        emit("    testq %%rax, %%rax");
        emit_i("    je .L%d", l_false);
        assignment_expr();
        emit_i("    jmp .L%d", l_end);
        emit_label(l_false);
        match(':');
        conditional_expr();
        emit_label(l_end);
        expr_pointed = 0;
    }
}

static void assignment_expr(void) {
    int saved_tok = tok;
    char saved_token[MAX_TOKEN_LEN];
    strcpy(saved_token, token);
    char *save_src = input_ptr;
    int save_line = line;

    if (tok != T_ID && tok != '*') {
        conditional_expr();
        return;
    }

    if (tok == T_ID) {
        next_token();
        char *peek_ptr = input_ptr;
        int peek_line = line;
        int peek_tok = tok;
        char peek_token[MAX_TOKEN_LEN];
        strcpy(peek_token, token);

        int assign_type = 0;
        while (tok == '[' || tok == '.' || tok == T_ARROW || tok == '(') {
            if (tok == '(') {
                int depth = 1;
                while (depth > 0 && tok != T_EOF) {
                    next_token();
                    if (tok == '(') depth++;
                    else if (tok == ')') depth--;
                }
                next_token();
            } else if (tok == '[') {
                int depth = 1;
                while (depth > 0 && tok != T_EOF) {
                    next_token();
                    if (tok == '[') depth++;
                    else if (tok == ']') depth--;
                }
                next_token();
            } else {
                next_token();
                next_token();
            }
        }
        if (tok == '=') assign_type = 1;
        else if (tok == T_INC) assign_type = 2;
        else if (tok == T_DEC) assign_type = 3;
        else if (tok == T_ADD_ASSIGN) assign_type = 4;
        else if (tok == T_SUB_ASSIGN) assign_type = 5;
        else assign_type = 0;

        input_ptr = peek_ptr;
        line = peek_line;
        tok = peek_tok;
        strcpy(token, peek_token);

        if (assign_type == 1) {
            tok = saved_tok; strcpy(token, saved_token);
            input_ptr = save_src; line = save_line;
            lvalue_address();
            match('=');
            emit("    pushq %%rax");
            assignment_expr();
            emit("    popq %%rcx");
            if (assign_size == 1) emit("    movb %%al, (%%rcx)");
            else emit("    movq %%rax, (%%rcx)");
            return;
        } else if (assign_type == 4) {
            tok = saved_tok; strcpy(token, saved_token);
            input_ptr = save_src; line = save_line;
            lvalue_address();
            emit("    pushq %%rax");
            if (assign_size == 1) emit("    movsbq (%%rax), %%rax");
            else emit("    movq (%%rax), %%rax");
            emit("    pushq %%rax");
            next_token();
            assignment_expr();
            emit("    popq %%rcx");
            if (assign_size == 1) {
                emit("    addq %%rcx, %%rax");
                emit("    popq %%rcx");
                emit("    movb %%al, (%%rcx)");
            } else {
                emit("    addq %%rcx, %%rax");
                emit("    popq %%rcx");
                emit("    movq %%rax, (%%rcx)");
            }
            return;
        } else if (assign_type == 5) {
            tok = saved_tok; strcpy(token, saved_token);
            input_ptr = save_src; line = save_line;
            lvalue_address();
            emit("    pushq %%rax");
            if (assign_size == 1) emit("    movsbq (%%rax), %%rax");
            else emit("    movq (%%rax), %%rax");
            emit("    pushq %%rax");
            next_token();
            assignment_expr();
            emit("    popq %%rcx");
            if (assign_size == 1) {
                emit("    subq %%rcx, %%rax");
                emit("    popq %%rcx");
                emit("    movb %%al, (%%rcx)");
            } else {
                emit("    subq %%rcx, %%rax");
                emit("    popq %%rcx");
                emit("    movq %%rax, (%%rcx)");
            }
            return;
        } else if (assign_type != 0) {
            int op = tok == T_INC ? T_INC : T_DEC;
            input_ptr = save_src; line = save_line;
            tok = saved_tok; strcpy(token, saved_token);
            lvalue_address();
            if (assign_size == 1) emit("    movsbq (%%rax), %%rcx");
            else emit("    movq (%%rax), %%rcx");
            if (op == T_INC) {
                if (assign_size == 1) emit("    addb $1, (%%rax)");
                else emit("    addq $1, (%%rax)");
            } else {
                if (assign_size == 1) emit("    subb $1, (%%rax)");
                else emit("    subq $1, (%%rax)");
            }
            emit("    movq %%rcx, %%rax");
            next_token();
            return;
        } else {
            tok = saved_tok; strcpy(token, saved_token);
            input_ptr = save_src; line = save_line;
            conditional_expr();
            return;
        }
    } else {
        char *peek_ptr = input_ptr;
        int peek_line = line;
        int peek_tok = tok;
        char peek_token[MAX_TOKEN_LEN];
        strcpy(peek_token, token);

        int saved_assign_size = assign_size;
        int saved_expr_pointed = expr_pointed;
        int saved_current_elem_size = current_elem_size;
        int saved_current_elem_size2 = current_elem_size2;
        int saved_no_postfix_deref = no_postfix_deref;

        peek_mode = 1;
        conditional_expr();
        peek_mode = 0;

        assign_size = saved_assign_size;
        expr_pointed = saved_expr_pointed;
        current_elem_size = saved_current_elem_size;
        current_elem_size2 = saved_current_elem_size2;
        no_postfix_deref = saved_no_postfix_deref;

        int assign_type = 0;
        if (tok == '=') assign_type = 1;
        else if (tok == T_INC) assign_type = 2;
        else if (tok == T_DEC) assign_type = 3;
        else if (tok == T_ADD_ASSIGN) assign_type = 4;
        else if (tok == T_SUB_ASSIGN) assign_type = 5;

        input_ptr = peek_ptr;
        line = peek_line;
        tok = peek_tok;
        strcpy(token, peek_token);

        if (assign_type == 1) {
            lvalue_address();
            match('=');
            emit("    pushq %%rax");
            assignment_expr();
            emit("    popq %%rcx");
            if (assign_size == 1) emit("    movb %%al, (%%rcx)");
            else emit("    movq %%rax, (%%rcx)");
            return;
        } else if (assign_type == 4) {
            lvalue_address();
            emit("    pushq %%rax");
            if (assign_size == 1) emit("    movsbq (%%rax), %%rax");
            else emit("    movq (%%rax), %%rax");
            emit("    pushq %%rax");
            next_token();
            assignment_expr();
            emit("    popq %%rcx");
            if (assign_size == 1) {
                emit("    addq %%rcx, %%rax");
                emit("    popq %%rcx");
                emit("    movb %%al, (%%rcx)");
            } else {
                emit("    addq %%rcx, %%rax");
                emit("    popq %%rcx");
                emit("    movq %%rax, (%%rcx)");
            }
            return;
        } else if (assign_type == 5) {
            lvalue_address();
            emit("    pushq %%rax");
            if (assign_size == 1) emit("    movsbq (%%rax), %%rax");
            else emit("    movq (%%rax), %%rax");
            emit("    pushq %%rax");
            next_token();
            assignment_expr();
            emit("    popq %%rcx");
            if (assign_size == 1) {
                emit("    subq %%rcx, %%rax");
                emit("    popq %%rcx");
                emit("    movb %%al, (%%rcx)");
            } else {
                emit("    subq %%rcx, %%rax");
                emit("    popq %%rcx");
                emit("    movq %%rax, (%%rcx)");
            }
            return;
        } else if (assign_type != 0) {
            lvalue_address();
            if (assign_size == 1) emit("    movsbq (%%rax), %%rcx");
            else emit("    movq (%%rax), %%rcx");
            if (assign_type == T_INC) {
                if (assign_size == 1) emit("    addb $1, (%%rax)");
                else emit("    addq $1, (%%rax)");
            } else {
                if (assign_size == 1) emit("    subb $1, (%%rax)");
                else emit("    subq $1, (%%rax)");
            }
            emit("    movq %%rcx, %%rax");
            next_token();
            return;
        } else {
            conditional_expr();
            return;
        }
    }
}

static void statement(void) {
    if (tok == T_IF) {
        next_token();
        match('(');
        assignment_expr();
        match(')');
        int l1 = label_counter++;
        int l2 = label_counter++;
        emit("    cmpq $0, %%rax");
        emit_i("    je .L%d", l1);
        statement();
        if (tok == T_ELSE) {
            next_token();
            emit_i("    jmp .L%d", l2);
            emit_label(l1);
            statement();
            emit_label(l2);
        } else {
            emit_label(l1);
        }
        return;
    }

    if (tok == T_FOR) {
        next_token();
        match('(');
        int saved_stack = stack_size;
        int saved_sym_count = symbol_count;

        if (tok == T_ID && (strcmp(token, "unsigned") == 0 ||
                            strcmp(token, "signed") == 0)) {
            next_token();
            while (tok == T_INT && (strcmp(token, "long") == 0 ||
                                    strcmp(token, "int") == 0)) next_token();
        }
        if (tok == T_INT || tok == T_CHAR) {
            int type = tok;
            next_token();
            while (tok == T_INT && (strcmp(token, "long") == 0 ||
                                    strcmp(token, "int") == 0)) next_token();

            while (tok != ';' && tok != T_EOF) {
                int is_ptr = 0;
                while (tok == '*') { is_ptr = 1; next_token(); }
                if (tok != T_ID) error("expected variable name");
                char varname[MAX_IDENT_LEN];
                size_t nlen = strlen(token);
                if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
                memcpy(varname, token, nlen);
                varname[nlen] = '\0';
                next_token();
                int vsize = is_ptr ? 8 : (type == T_INT ? 8 : 1);
                add_symbol(varname, 0, vsize, is_ptr ? type : 0, 0, 0);
                if (tok == '=') {
                    next_token();
                    assignment_expr();
                    int si = find_symbol(varname);
                    Symbol *s = &symbols[si];
                    if (s->is_global) emit_s("    movq %%rax, %s(%%rip)", s->name);
                    else emit_i("    movq %%rax, %d(%%rbp)", s->offset);
                }
                if (tok == ',') next_token();
                else break;
            }
            match(';');
        } else if (tok != ';') {
            while (tok != ';' && tok != T_EOF) {
                assignment_expr();
                if (tok == ',') next_token();
                else break;
            }
            match(';');
        } else {
            match(';');
        }

        char *cond_pos = input_ptr;
        int cond_line_saved = line;
        int cond_first_tok = tok;
        char cond_first_tok_val[MAX_TOKEN_LEN];
        strcpy(cond_first_tok_val, token);
        int has_cond = (tok != ';');
        if (has_cond) {
            while (tok != ';' && tok != T_EOF) next_token();
        }
        match(';');

        char *inc_pos = input_ptr;
        int inc_line_saved = line;
        int inc_first_tok = tok;
        char inc_first_tok_val[MAX_TOKEN_LEN];
        strcpy(inc_first_tok_val, token);
        int has_inc = (tok != ')');
        if (has_inc) {
            while (tok != ')' && tok != T_EOF) next_token();
        }
        match(')');

        int l_body = label_counter++;
        int l_inc = has_inc ? label_counter++ : 0;
        int l_cond = label_counter++;
        int l_end = label_counter++;
        int l_cont = has_inc ? l_inc : l_cond;

        emit_i("    jmp .L%d", l_cond);
        emit_label(l_body);

        {
            int saved_cont = continue_target;
            int saved_cont_valid = continue_target_valid;
            int saved_break = break_target;
            int saved_break_valid = break_target_valid;
            continue_target = l_cont;
            continue_target_valid = 1;
            break_target = l_end;
            break_target_valid = 1;
            statement();
            continue_target = saved_cont;
            continue_target_valid = saved_cont_valid;
            break_target = saved_break;
            break_target_valid = saved_break_valid;
        }

        char *after_body_pos = input_ptr;
        int after_body_line = line;
        int after_body_tok = tok;
        char after_body_token[MAX_TOKEN_LEN];
        strcpy(after_body_token, token);

        if (has_inc) {
            emit_label(l_inc);
            input_ptr = inc_pos; line = inc_line_saved;
            tok = inc_first_tok; strcpy(token, inc_first_tok_val);

            ParserState saved_state;
            save_parser_state(&saved_state);

            int paren_depth = 0;
            while (tok != T_EOF) {
                if (tok == '(') paren_depth++;
                else if (tok == ')') {
                    if (paren_depth == 0) break;
                    paren_depth--;
                }
                assignment_expr();
                if (tok == ',') {
                    next_token();
                } else if (tok == ')' && paren_depth == 0) {
                    break;
                } else if (tok != ')') {
                    break;
                }
            }
            restore_parser_state(&saved_state);
            emit_i("    jmp .L%d", l_cond);
        }

        emit_label(l_cond);
        if (has_cond) {
            input_ptr = cond_pos; line = cond_line_saved;
            tok = cond_first_tok; strcpy(token, cond_first_tok_val);

            ParserState saved_state;
            save_parser_state(&saved_state);

            int semicolon_found = 0;
            int paren_depth = 0;
            while (tok != T_EOF && !semicolon_found) {
                if (tok == '(') paren_depth++;
                else if (tok == ')') paren_depth--;
                else if (tok == ';' && paren_depth == 0) {
                    semicolon_found = 1;
                    break;
                }
                assignment_expr();
                if (tok == ',') {
                    next_token();
                } else if (tok == ';' && paren_depth == 0) {
                    break;
                }
            }
            restore_parser_state(&saved_state);
            emit("    cmpq $0, %%rax");
            emit_i("    jne .L%d", l_body);
        } else {
            emit_i("    jmp .L%d", l_body);
        }

        emit_label(l_end);

        input_ptr = after_body_pos; line = after_body_line;
        tok = after_body_tok; strcpy(token, after_body_token);
        stack_size = saved_stack;
        symbol_count = saved_sym_count;
        return;
    }

    if (tok == T_WHILE) {
        next_token();
        match('(');
        int l1 = label_counter++;
        int l2 = label_counter++;
        int saved_cont = continue_target;
        int saved_cont_valid = continue_target_valid;
        int saved_break = break_target;
        int saved_break_valid = break_target_valid;
        continue_target = l1;
        continue_target_valid = 1;
        break_target = l2;
        break_target_valid = 1;
        emit_label(l1);
        assignment_expr();
        match(')');
        emit("    cmpq $0, %%rax");
        emit_i("    je .L%d", l2);
        statement();
        emit_i("    jmp .L%d", l1);
        emit_label(l2);
        continue_target = saved_cont;
        continue_target_valid = saved_cont_valid;
        break_target = saved_break;
        break_target_valid = saved_break_valid;
        return;
    }

    if (tok == T_SWITCH) {
        next_token();
        match('(');
        assignment_expr();
        match(')');
        int dispatch_label = label_counter++;
        int end_label = label_counter++;

        emit("    pushq %%rax");
        emit("    pushq $0");
        emit_i("    jmp .L%d", dispatch_label);

        int saved_break_target = break_target;
        int saved_break_valid = break_target_valid;
        int saved_case_count = switch_case_count;
        int saved_has_default = switch_has_default;
        int saved_default_label = switch_default_label;
        break_target = end_label;
        break_target_valid = 1;
        switch_case_count = 0;
        switch_has_default = 0;
        switch_default_label = 0;

        match('{');
        while (tok != '}' && tok != T_EOF) {
            if (tok == T_CASE) {
                next_token();
                int val = atoi(token);
                next_token();
                match(':');
                int lbl = label_counter++;
                switch_case_values[switch_case_count] = val;
                switch_case_labels[switch_case_count] = lbl;
                switch_case_count++;
                emit_label(lbl);
            } else if (tok == T_DEFAULT) {
                next_token();
                match(':');
                switch_has_default = 1;
                switch_default_label = label_counter++;
                emit_label(switch_default_label);
            } else {
                statement();
            }
        }
        match('}');

        emit_i("    jmp .L%d", end_label);

        emit_label(dispatch_label);
        emit("    movq 8(%%rsp), %%rax");
        for (int i = 0; i < switch_case_count; i++) {
            emit_i("    cmpq $%d, %%rax", switch_case_values[i]);
            emit_i("    je .L%d", switch_case_labels[i]);
        }
        if (switch_has_default)
            emit_i("    jmp .L%d", switch_default_label);
        emit_label(end_label);
        emit("    addq $16, %%rsp");

        break_target = saved_break_target;
        break_target_valid = saved_break_valid;
        switch_case_count = saved_case_count;
        switch_has_default = saved_has_default;
        switch_default_label = saved_default_label;
        return;
    }

    if (tok == T_BREAK) {
        next_token();
        match(';');
        if (break_target_valid)
            emit_i("    jmp .L%d", break_target);
        return;
    }

    if (tok == T_CONTINUE) {
        next_token();
        match(';');
        if (continue_target_valid)
            emit_i("    jmp .L%d", continue_target);
        return;
    }

    if (tok == T_GOTO) {
        next_token();
        if (tok != T_ID) error("expected label name");
        emit_s("    jmp %s", token);
        next_token();
        match(';');
        return;
    }

    if (tok == T_RETURN) {
        function_has_return = 1;
        next_token();
        if (tok != ';') assignment_expr();
        match(';');
        emit("    leave");
        emit("    ret");
        return;
    }

    if (tok == '{') {
        next_token();
        int saved_stack = stack_size;
        int saved_sym_count = symbol_count;
        while (tok != '}' && tok != T_EOF) {
            if (tok == T_ID && (strcmp(token, "unsigned") == 0 ||
                                strcmp(token, "signed") == 0)) {
                next_token();
            }
            if (tok == T_STATIC) {
                next_token();
                continue;
            } else if (tok == T_CONST) {
                next_token();
                continue;
            } else if (tok == T_TYPEDEF) {
                skip_typedef();
            } else if (tok == T_STRUCT) {
                next_token();
                skip_struct();
            } else if (tok == T_ID) {
                const char *p = input_ptr;
                while (*p && (*p == ' ' || *p == '\t')) p++;
                if (*p == ':') {
                    emit_s("%s:", token);
                    next_token();
                    next_token();
                    continue;
                }
                int ti = find_symbol(token);
                if (ti < 0 || !symbols[ti].is_const) {
                    statement();
                    continue;
                }
                int type_size = symbols[ti].const_value;
                next_token();
            restart_typedef:
                int is_ptr = 0;
                while (tok == '*') { is_ptr = 1; next_token(); }
                if (tok != T_ID) error("expected variable name");
                char varname[MAX_IDENT_LEN];
                size_t nlen = strlen(token);
                if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
                memcpy(varname, token, nlen);
                varname[nlen] = '\0';
                next_token();
                int size = is_ptr ? 8 : type_size;
                int is_arr = 0;
                int elem_size = size;
                int elem_size2 = 0;
                int ndims = 0;
                while (tok == '[') {
                    is_arr = 1;
                    next_token();
                    int cnt = 0;
                    if (tok == T_NUM) {
                        cnt = atoi(token);
                        next_token();
                    } else if (tok == T_ID) {
                        int mi = find_macro(token);
                        if (mi >= 0) cnt = macros[mi].value;
                        else error("undefined macro");
                        next_token();
                    }
                    match(']');
                    size = size * (cnt > 0 ? cnt : 1);
                    ndims++;
                    if (ndims == 2) {
                        elem_size2 = elem_size;
                        elem_size = elem_size * (cnt > 0 ? cnt : 1);
                    }
                }
                add_symbol(varname, 0, size, is_ptr ? T_INT : 0, is_arr, elem_size);
                if (elem_size2 > 0) {
                    Symbol *s2 = &symbols[symbol_count - 1];
                    s2->elem_size2 = elem_size2;
                }
                if (tok == '=') {
                    next_token();
                    if (tok == '{') {
                        next_token();
                        int elem_idx = 0;
                        while (tok != '}') {
                            assignment_expr();
                            int si = find_symbol(varname);
                            Symbol *s = &symbols[si];
                            int off = s->offset + elem_idx * elem_size;
                            if (elem_size == 1)
                                emit_i("    movb %%al, %d(%%rbp)", off);
                            else
                                emit_i("    movq %%rax, %d(%%rbp)", off);
                            if (tok == ',') next_token();
                            elem_idx++;
                        }
                        match('}');
                        if (is_arr && size == 0) {
                            size = elem_idx * elem_size;
                            int si = find_symbol(varname);
                            Symbol *s = &symbols[si];
                            int old_al = (s->size + STACK_ALIGN - 1) & ~(STACK_ALIGN - 1);
                            int new_al = (size + STACK_ALIGN - 1) & ~(STACK_ALIGN - 1);
                            s->size = size;
                            s->offset -= (new_al - old_al);
                            stack_size += (new_al - old_al);
                            if (stack_size > max_func_stack)
                                max_func_stack = stack_size;
                        }
                    } else {
                        assignment_expr();
                        int si = find_symbol(varname);
                        Symbol *s = &symbols[si];
                        if (s->is_global)
                            emit_s("    movq %%rax, %s(%%rip)", s->name);
                        else
                            emit_i("    movq %%rax, %d(%%rbp)", s->offset);
                    }
                }
                if (tok == ',') {
                    next_token();
                    goto restart_typedef;
                }
                match(';');
            } else if (tok == T_INT || tok == T_CHAR || tok == T_VOID) {
                int type = tok;
                int ndims = 0;
                next_token();
                while (tok == T_INT && (strcmp(token, "long") == 0 ||
                                        strcmp(token, "int") == 0)) next_token();
            restart_int:
                int is_ptr = 0;
                while (tok == '*') { is_ptr = 1; next_token(); }
                if (tok != T_ID) error("expected variable name");
                char varname[MAX_IDENT_LEN];
                size_t nlen = strlen(token);
                if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
                memcpy(varname, token, nlen);
                varname[nlen] = '\0';
                next_token();
                int gsize = is_ptr ? 8 : (type == T_CHAR ? 1 : 8);
                int is_arr = 0;
                int elem_size = gsize;
                int elem_size2 = 0;
                while (tok == '[') {
                    is_arr = 1;
                    next_token();
                    int cnt = 0;
                    if (tok == T_NUM) {
                        cnt = atoi(token);
                        next_token();
                    } else if (tok == T_ID) {
                        int mi = find_macro(token);
                        if (mi >= 0) cnt = macros[mi].value;
                        else error("undefined macro");
                        next_token();
                    }
                    match(']');
                    gsize = gsize * (cnt > 0 ? cnt : 1);
                    ndims++;
                    if (ndims == 2) {
                        elem_size2 = elem_size;
                        elem_size = elem_size * (cnt > 0 ? cnt : 1);
                    }
                }
                add_symbol(varname, 0, gsize, is_ptr ? type : 0, is_arr, elem_size);
                if (elem_size2 > 0) {
                    Symbol *s2 = &symbols[symbol_count - 1];
                    s2->elem_size2 = elem_size2;
                }
                if (tok == '=') {
                    next_token();
                    if (tok == '{') {
                        next_token();
                        int elem_idx = 0;
                        while (tok != '}') {
                            assignment_expr();
                            int si = find_symbol(varname);
                            Symbol *s = &symbols[si];
                            int off = s->offset + elem_idx * elem_size;
                            if (elem_size == 1)
                                emit_i("    movb %%al, %d(%%rbp)", off);
                            else
                                emit_i("    movq %%rax, %d(%%rbp)", off);
                            if (tok == ',') next_token();
                            elem_idx++;
                        }
                        match('}');
                        if (is_arr && gsize == 0) {
                            gsize = elem_idx * elem_size;
                            int si = find_symbol(varname);
                            Symbol *s = &symbols[si];
                            int old_al = (s->size + STACK_ALIGN - 1) & ~(STACK_ALIGN - 1);
                            int new_al = (gsize + STACK_ALIGN - 1) & ~(STACK_ALIGN - 1);
                            s->size = gsize;
                            s->offset -= (new_al - old_al);
                            stack_size += (new_al - old_al);
                            if (stack_size > max_func_stack)
                                max_func_stack = stack_size;
                        }
                    } else {
                        assignment_expr();
                        int si = find_symbol(varname);
                        Symbol *s = &symbols[si];
                        if (s->is_global)
                            emit_s("    movq %%rax, %s(%%rip)", s->name);
                        else
                            emit_i("    movq %%rax, %d(%%rbp)", s->offset);
                    }
                }
                if (tok == ',') {
                    next_token();
                    goto restart_int;
                }
                match(';');
            } else if (tok == T_ENUM) {
                parse_enum();
            } else {
                statement();
            }
        }
        match('}');
        stack_size = saved_stack;
        symbol_count = saved_sym_count;
        return;
    }

    if (tok == '(') {
        char *peek_src = input_ptr;
        int peek_line = line;
        int peek_tok = tok;
        char peek_token[MAX_TOKEN_LEN];
        strcpy(peek_token, token);
        next_token();
        if (tok == T_VOID) {
            next_token();
            if (tok == ')') {
                next_token();
                assignment_expr();
                match(';');
                return;
            }
        }
        input_ptr = peek_src;
        line = peek_line;
        tok = peek_tok;
        strcpy(token, peek_token);
    }

    if (tok == ';') {
        next_token();
        return;
    }

    assignment_expr();
    match(';');
}

static void parse_function(const char *name, int ret_type) {
    (void)ret_type;
    int outer_stack = stack_size;
    int saved_sym_count = symbol_count;
    int saved_stack_outer = stack_size;

    match('(');

    char param_names[MAX_SYMBOLS][MAX_IDENT_LEN];
    int param_count = 0;
    if (tok == T_VOID) {
        next_token();
    } else {
        while (tok != ')' && tok != T_EOF) {
            if (tok == T_CONST) { next_token(); continue; }
            if (tok == T_ID && (strcmp(token, "unsigned") == 0 ||
                                strcmp(token, "signed") == 0)) {
                next_token();
                while (tok == T_INT && (strcmp(token, "long") == 0 ||
                                        strcmp(token, "int") == 0)) next_token();
            }
            if (tok == T_INT || tok == T_CHAR || tok == T_VOID || tok == T_ID) {
                int ptype = (tok == T_ID) ? T_INT : tok;
                if (tok == T_ID) {
                    int ti = find_symbol(token);
                    if (ti >= 0 && symbols[ti].is_const) ptype = T_INT;
                }
                next_token();
                while (tok == T_INT && (strcmp(token, "long") == 0 ||
                                        strcmp(token, "int") == 0)) next_token();
                int nstars = 0;
                while (tok == '*') { nstars++; next_token(); }
                int is_ptr = nstars > 0;
                if (tok != T_ID) error("expected parameter name");
                size_t nlen = strlen(token);
                if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
                memcpy(param_names[param_count], token, nlen);
                param_names[param_count][nlen] = '\0';
                int psize = is_ptr ? 8 : (ptype == T_CHAR ? 1 : 8);
                int pointed_type = 0;
                if (nstars == 1) pointed_type = ptype;
                else if (nstars > 1) pointed_type = T_INT;
                add_symbol(token, 0, psize, pointed_type, 0, 0);
                param_count++;
                next_token();
                if (tok == ',') next_token();
            } else if (tok == '.') {
                next_token(); if (tok == '.') next_token();
                if (tok == '.') next_token();
                break;
            } else {
                next_token();
            }
        }
    }
    match(')');

    if (tok == ';') {
        symbol_count = saved_sym_count;
        stack_size = saved_stack_outer;
        next_token();
        return;
    }

    if (tok != '{') error("expected function body");

    int param_stack = stack_size - outer_stack;

    char *body_start = input_ptr;
    int body_line = line;
    int body_tok = tok;
    char body_token[MAX_TOKEN_LEN];
    strcpy(body_token, token);
    int saved_symbol_cnt = symbol_count;

    emit_enabled = 0;
    max_func_stack = param_stack;
    stack_size = param_stack;
    function_has_return = 0;

    statement();

    int func_stack = (max_func_stack + STACK_ALIGN - 1) & ~(STACK_ALIGN - 1);
    if (func_stack < 64) func_stack = 64;

    symbol_count = saved_symbol_cnt;
    input_ptr = body_start;
    line = body_line;
    tok = body_tok;
    strcpy(token, body_token);
    emit_enabled = 1;

    emit_s("    .globl %s", name);
    emit_s("%s:", name);
    emit("    pushq %%rbp");
    emit("    movq %%rsp, %%rbp");
    if (func_stack > 0)
        emit_i("    subq $%d, %%rsp", func_stack);
    int safe_stack = (func_stack + 16 + STACK_ALIGN - 1) & ~(STACK_ALIGN - 1);
    if (safe_stack > 0)
        emit_i("    subq $%d, %%rsp", safe_stack);

    for (int i = 0; i < param_count && i < 6; i++) {
        int idx = find_symbol(param_names[i]);
        if (idx >= 0) {
            Symbol *s = &symbols[idx];
            emit_si("    movq %s, %d(%%rbp)", arg_reg(i), s->offset);
        }
    }

    stack_size = param_stack;
    function_has_return = 0;
    statement();

    emit("    leave");
    emit("    ret");

    stack_size = outer_stack;
}

static void parse_enum(void) {
    next_token();
    if (tok == T_ID) {
        next_token();
    }
    if (tok != '{') error("expected '{' after enum");
    next_token();
    int val = 0;
    while (tok != '}' && tok != T_EOF) {
        if (tok != T_ID) error("expected enumerator name");
        if (symbol_count >= MAX_SYMBOLS) error("too many symbols");
        Symbol *s = &symbols[symbol_count];
        symbol_count++;
        size_t nlen = strlen(token);
        if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
        memcpy(s->name, token, nlen);
        s->name[nlen] = '\0';
        s->is_const = 1;
        s->is_global = 0;
        s->size = 8;
        s->pointed = 0;
        s->elem_size2 = 0;
        next_token();
        if (tok == '=') {
            next_token();
            if (tok != T_NUM) error("expected integer constant");
            val = atoi(token);
            next_token();
        }
        s->const_value = val;
        val++;
        if (tok == ',') next_token();
    }
    match('}');
    match(';');
}

static void skip_struct(void) {
    if (tok == T_ID) next_token();
    if (tok != '{') error("expected '{' in struct");
    next_token();

    struct_total_size = 0;

    while (tok != '}' && tok != T_EOF) {
        if (tok == T_INT || tok == T_CHAR) {
            int ftype = tok;
            next_token();
            int is_ptr = 0;
            while (tok == '*') { is_ptr = 1; next_token(); }
            int fsize = is_ptr ? 8 : (ftype == T_CHAR ? 1 : 8);
            while (tok != ';' && tok != '}' && tok != T_EOF) {
                if (tok == T_ID) {
                    if (struct_member_count < MAX_STRUCT_MEMBERS) {
                        size_t nlen = strlen(token);
                        if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
                        memcpy(struct_member_names[struct_member_count], token, nlen);
                        struct_member_names[struct_member_count][nlen] = '\0';
                        struct_member_offsets[struct_member_count] = struct_total_size;
                        struct_member_sizes[struct_member_count] = fsize;
                        struct_member_elem_sizes[struct_member_count] = fsize;
                        struct_member_count++;
                    }
                    next_token();
                    if (tok == '[') {
                        next_token();
                        int count = 1;
                        if (tok == T_NUM) { count = atoi(token); next_token(); }
                        match(']');
                        struct_total_size += fsize * count;
                        if (struct_member_count > 0) {
                            struct_member_sizes[struct_member_count - 1] =
                                fsize * count;
                        }
                    } else {
                        struct_total_size += fsize;
                    }
                } else {
                    next_token();
                }
            }
            match(';');
        } else if (tok == '}') {
            break;
        } else {
            next_token();
        }
    }
    match('}');
}

static void skip_typedef(void) {
    next_token();
    if (tok == T_STRUCT) {
        next_token();
        skip_struct();
    } else if (tok == T_INT || tok == T_CHAR || tok == T_VOID) {
        next_token();
        while (tok == '*') next_token();
        if (tok == T_ID) next_token();
    }
    char last_name[MAX_IDENT_LEN] = "";
    while (tok != ';' && tok != T_EOF) {
        if (tok == T_ID) {
            size_t nlen = strlen(token);
            if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
            memcpy(last_name, token, nlen);
            last_name[nlen] = '\0';
        }
        next_token();
    }
    if (last_name[0]) {
        if (symbol_count >= MAX_SYMBOLS) error("too many symbols");
        Symbol *s = &symbols[symbol_count];
        symbol_count++;
        size_t nlen = strlen(last_name);
        if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
        memcpy(s->name, last_name, nlen);
        s->name[nlen] = '\0';
        s->is_const = 1;
        s->is_global = 0;
        s->size = 8;
        s->pointed = 0;
        s->elem_size2 = 0;
        s->const_value = (struct_total_size > 0) ? struct_total_size : 8;
    }
    match(';');
}

static void parse_program(void) {
    next_token();
    while (tok != T_EOF) {
        if (tok == T_ID && (strcmp(token, "unsigned") == 0 ||
                            strcmp(token, "signed") == 0)) {
            next_token();
            while (tok == T_INT && (strcmp(token, "long") == 0 ||
                                    strcmp(token, "int") == 0)) next_token();
        }
        if (tok == T_STATIC) {
            next_token();
            continue;
        } else if (tok == T_CONST) {
            next_token();
            continue;
        } else if (tok == T_TYPEDEF) {
            skip_typedef();
        } else if (tok == T_STRUCT) {
            next_token();
            skip_struct();
        } else if (tok == T_ENUM) {
            parse_enum();
        } else if (tok == T_INT || tok == T_CHAR || tok == T_VOID) {
            int type = tok;
            next_token();
            while (tok == T_INT && (strcmp(token, "long") == 0 ||
                                    strcmp(token, "int") == 0)) next_token();
            int is_ptr = 0;
            while (tok == '*') { is_ptr = 1; next_token(); }
            if (tok != T_ID) error("expected identifier");
            char fname[MAX_IDENT_LEN];
            size_t nlen = strlen(token);
            if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
            memcpy(fname, token, nlen);
            fname[nlen] = '\0';
            next_token();
            if (tok == '(') {
                parse_function(fname, type);
            } else {
                int gsize = is_ptr ? 8 : (type == T_CHAR ? 1 : 8);
                int is_arr = 0;
                int elem_size = gsize;
                int elem_size2 = 0;
                int ndims = 0;
                while (tok == '[') {
                    is_arr = 1;
                    next_token();
                    int cnt = 0;
                    if (tok == T_NUM) {
                        cnt = atoi(token);
                        next_token();
                    } else if (tok == T_ID) {
                        int mi = find_macro(token);
                        if (mi >= 0) cnt = macros[mi].value;
                        else error("undefined macro");
                        next_token();
                    }
                    match(']');
                    gsize = gsize * (cnt > 0 ? cnt : 1);
                    ndims++;
                    if (ndims == 2) {
                        elem_size2 = elem_size;
                        elem_size = elem_size * (cnt > 0 ? cnt : 1);
                    }
                }
                add_symbol(fname, 1, gsize, is_ptr ? type : 0, is_arr, elem_size);
                if (elem_size2 > 0) {
                    Symbol *s2 = &symbols[symbol_count - 1];
                    s2->elem_size2 = elem_size2;
                }
                if (tok == '=') {
                    while (tok != ';' && tok != T_EOF) next_token();
                }
                if (tok == ';') next_token();
                else error("expected ';' or '(' after global");
            }
        } else if (tok == T_ID) {
            int type_size = 8;
            int ti = find_symbol(token);
            if (ti >= 0 && symbols[ti].is_const) type_size = symbols[ti].const_value;
            next_token();
            int is_ptr = 0;
            while (tok == '*') { is_ptr = 1; next_token(); }
            if (tok != T_ID) error("expected identifier");
            char fname[MAX_IDENT_LEN];
            size_t nlen = strlen(token);
            if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
            memcpy(fname, token, nlen);
            fname[nlen] = '\0';
            next_token();
            int gsize = is_ptr ? 8 : type_size;
            int is_arr = 0;
            int elem_size = gsize;
            int elem_size2 = 0;
            int ndims = 0;
            while (tok == '[') {
                is_arr = 1;
                next_token();
                int cnt = 0;
                if (tok == T_NUM) {
                    cnt = atoi(token);
                    next_token();
                } else if (tok == T_ID) {
                    int mi = find_macro(token);
                    if (mi >= 0) cnt = macros[mi].value;
                    else error("undefined macro");
                    next_token();
                }
                match(']');
                gsize = gsize * (cnt > 0 ? cnt : 1);
                ndims++;
                if (ndims == 2) {
                    elem_size2 = elem_size;
                    elem_size = elem_size * (cnt > 0 ? cnt : 1);
                }
            }
            add_symbol(fname, 1, gsize, 0, is_arr, elem_size);
            if (elem_size2 > 0) {
                Symbol *s2 = &symbols[symbol_count - 1];
                s2->elem_size2 = elem_size2;
            }
            if (tok == ';') next_token();
            else error("expected ';' or '(' after global");
        } else {
            error("global must be int or char");
        }
    }
}

static void emit_string_pool(void) {
    if (!emit_enabled) return;
    for (int i = 0; i < string_count; i++) {
        fprintf(output, ".Lstr%d:\n    .asciz \"", i);
        const char *s = string_pool[i];
        while (*s) {
            unsigned char c = *s;
            if (c == '\n') fprintf(output, "\\n");
            else if (c == '\t') fprintf(output, "\\t");
            else if (c == '\\') fprintf(output, "\\\\");
            else if (c == '"') fprintf(output, "\\\"");
            else if (c >= 32 && c < 127) fputc(c, output);
            else fprintf(output, "\\%hho", c);
            s++;
        }
        fprintf(output, "\"\n");
        free(string_pool[i]);
    }
}

/*
 * Main entry point – can act as compiler or hotswap loader.
 */
int main(int argc, char **argv) {
    /* Hotswap mode: --runelf <filename> [funcname] [args] */
    if (argc >= 3 && strcmp(argv[1], "--runelf") == 0) {
        const char *func = (argc > 3) ? argv[3] : "go";
        const char *args = (argc > 4) ? argv[4] : "";
        return run_elf_from_file(argv[2], func, args);
    }

    /* Original compiler mode */
    emit_enabled = 1;
    line = 1;
    assign_size = 8;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s source.c > output.s\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    fseek(input, 0, SEEK_END);
    long file_size = ftell(input);
    if (file_size < 0 || file_size > MAX_SOURCE_SIZE) {
        fprintf(stderr, "Invalid file size\n");
        fclose(input);
        return EXIT_FAILURE;
    }
    rewind(input);

    source_start = safe_malloc((size_t)file_size + 1);
    size_t read_bytes = fread(source_start, 1, (size_t)file_size, input);
    if (read_bytes != (size_t)file_size) {
        fprintf(stderr, "Error reading file\n");
        free(source_start);
        fclose(input);
        return EXIT_FAILURE;
    }
    source_start[file_size] = '\0';
    fclose(input);

    input_ptr = source_start;
    output = stdout;

    add_macro("EXIT_FAILURE", 1);
    add_macro("EXIT_SUCCESS", 0);
    add_macro("SEEK_SET", 0);
    add_macro("SEEK_CUR", 1);
    add_macro("SEEK_END", 2);
    add_macro("NULL", 0);

    /* Predefine libc globals. */
    for (int i = 0; ; i++) {
        const char *gname = libc_global_name(i);
        if (gname == NULL) break;
        if (symbol_count >= MAX_SYMBOLS) error("too many symbols");
        Symbol *s = &symbols[symbol_count];
        symbol_count++;
        size_t nlen = strlen(gname);
        if (nlen >= MAX_IDENT_LEN) nlen = MAX_IDENT_LEN - 1;
        memcpy(s->name, gname, nlen);
        s->name[nlen] = '\0';
        s->offset = 0;
        s->is_global = 1;
        s->size = 8;
        s->pointed = 0;
        s->is_const = (strcmp(gname, "size_t") == 0 ||
                       strcmp(gname, "va_list") == 0 ||
                       strcmp(gname, "FILE") == 0) ? 1 : 0;
        s->const_value = 8;
        s->is_array = 0;
        s->elem_size = 0;
        s->elem_size2 = 0;
    }

    emit("    .section .text");
    parse_program();

    if (string_count > 0) {
        emit("    .section .rodata");
        emit_string_pool();
        emit("    .section .text");
    }

    emit("    .globl _start");
    /* Optionally add _start stub if not provided */
    /* The original compiler omitted _start; we keep it as is. */

    free(source_start);
    return EXIT_SUCCESS;
}