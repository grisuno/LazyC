# API

## beacon.h

### BeaconDataParse (function) `void BeaconDataParse(datap *parser, char *buffer, int size);`
- Defined: `beacon.h:21`
- Doc: === API para BOFs ===

### BeaconDataPtr (function) `char *BeaconDataPtr(datap *parser, int size);`
- Defined: `beacon.h:22`

### BeaconDataInt (function) `int BeaconDataInt(datap *parser);`
- Defined: `beacon.h:23`

### BeaconDataShort (function) `short BeaconDataShort(datap *parser);`
- Defined: `beacon.h:24`

### BeaconDataLength (function) `int BeaconDataLength(datap *parser);`
- Defined: `beacon.h:25`

### BeaconDataExtract (function) `char *BeaconDataExtract(datap *parser, int *size);`
- Defined: `beacon.h:26`

### BeaconPrintf (function) `void BeaconPrintf(int type, const char *fmt, ...);`
- Defined: `beacon.h:27`

### BeaconOutput (function) `void BeaconOutput(int type, const char *data, int len);`
- Defined: `beacon.h:28`

## lazyc.c

### BeaconPrintf (function) `static void BeaconPrintf(int type, const char *fmt, ...)`
- Defined: `lazyc.c:177`
- Doc: Beacon API – capture output into global buffer.

### BeaconOutput (function) `static void BeaconOutput(int type, const char *data, int len)`
- Defined: `lazyc.c:192`

### create_trampoline (function) `static void *create_trampoline(void *target)`
- Defined: `lazyc.c:209`
- Doc: Trampoline creation and management.

### cleanup_trampolines (function) `static void cleanup_trampolines(void)`
- Defined: `lazyc.c:240`

### get_or_create_trampoline (function) `static void *get_or_create_trampoline(void *target)`
- Defined: `lazyc.c:255`

### page_align (function) `static size_t page_align(size_t size)`
- Defined: `lazyc.c:291`
- Doc: Page alignment utility.

### call_bof_isolated (function) `static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t arglen)`
- Defined: `lazyc.c:303`
- Doc: Isolated call to BOF function with correct stack alignment.

### resolve_external_symbols (function) `static int resolve_external_symbols(void)`
- Defined: `lazyc.c:335`
- Doc: Resolve all external symbols needed by the BOF.

### run_elf (function) `static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t filesize,
        ...`
- Defined: `lazyc.c:353`
- Doc: Core ELF loader: maps sections, resolves relocations, executes entry point.

### run_elf_from_file (function) `static int run_elf_from_file(const char *filename, const char *funcname,
                        ...`
- Defined: `lazyc.c:612`
- Doc: Convenience wrapper: load ELF from file and run.

### error (function) `static void error(const char *msg)`
- Defined: `lazyc.c:846`
- Doc: Error handling.

### safe_malloc (function) `static void *safe_malloc(size_t size)`
- Defined: `lazyc.c:850`

### my_isspace (function) `static int my_isspace(int c)`
- Defined: `lazyc.c:859`

### my_isalpha (function) `static int my_isalpha(int c)`
- Defined: `lazyc.c:864`

### my_isdigit (function) `static int my_isdigit(int c)`
- Defined: `lazyc.c:868`

### my_isalnum (function) `static int my_isalnum(int c)`
- Defined: `lazyc.c:872`

### find_macro (function) `static int find_macro(const char *name)`
- Defined: `lazyc.c:876`

### add_macro (function) `static void add_macro(const char *name, int value)`
- Defined: `lazyc.c:884`

### save_parser_state (function) `static void save_parser_state(ParserState *state)`
- Defined: `lazyc.c:895`

### restore_parser_state (function) `static void restore_parser_state(ParserState *state)`
- Defined: `lazyc.c:919`

### next_token (function) `static void next_token(void)`
- Defined: `lazyc.c:943`

### match (function) `static void match(int expected)`
- Defined: `lazyc.c:1168`

### emit (function) `static void emit(const char *s)`
- Defined: `lazyc.c:1173`

### emit_i (function) `static void emit_i(const char *fmt, int v)`
- Defined: `lazyc.c:1187`

### emit_s (function) `static void emit_s(const char *fmt, const char *s)`
- Defined: `lazyc.c:1193`

### emit_is (function) `static void emit_is(const char *fmt, int v, const char *s)`
- Defined: `lazyc.c:1199`

### emit_si (function) `static void emit_si(const char *fmt, const char *s, int v)`
- Defined: `lazyc.c:1205`

### emit_label (function) `static void emit_label(int label)`
- Defined: `lazyc.c:1211`

### find_symbol (function) `static int find_symbol(const char *name)`
- Defined: `lazyc.c:1216`

### add_symbol (function) `static void add_symbol(const char *name, int is_global, int size, int pointed,
                  ...`
- Defined: `lazyc.c:1224`

### arg_reg (function) `static const char *arg_reg(int i)`
- Defined: `lazyc.c:1258`

### libc_global_name (function) `static const char *libc_global_name(int i)`
- Defined: `lazyc.c:1267`

### unary (function) `static void unary(void)`
- Defined: `lazyc.c:1280`

### lvalue_address (function) `static void lvalue_address(void)`
- Defined: `lazyc.c:1434`

### handle_postfix (function) `static void handle_postfix(int is_lvalue)`
- Defined: `lazyc.c:1479`

### unary_expr (function) `static void unary_expr(void)`
- Defined: `lazyc.c:1580`

### multiplicative_expr (function) `static void multiplicative_expr(void)`
- Defined: `lazyc.c:1585`

### additive_expr (function) `static void additive_expr(void)`
- Defined: `lazyc.c:1611`

### relational_expr (function) `static void relational_expr(void)`
- Defined: `lazyc.c:1629`

### equality_expr (function) `static void equality_expr(void)`
- Defined: `lazyc.c:1647`

### bitwise_and_expr (function) `static void bitwise_and_expr(void)`
- Defined: `lazyc.c:1663`

### bitwise_xor_expr (function) `static void bitwise_xor_expr(void)`
- Defined: `lazyc.c:1675`

### bitwise_or_expr (function) `static void bitwise_or_expr(void)`
- Defined: `lazyc.c:1687`

### logical_and_expr (function) `static void logical_and_expr(void)`
- Defined: `lazyc.c:1699`

### logical_or_expr (function) `static void logical_or_expr(void)`
- Defined: `lazyc.c:1719`

### conditional_expr (function) `static void conditional_expr(void)`
- Defined: `lazyc.c:1739`

### assignment_expr (function) `static void assignment_expr(void)`
- Defined: `lazyc.c:1757`

### statement (function) `static void statement(void)`
- Defined: `lazyc.c:1991`

### parse_function (function) `static void parse_function(const char *name, int ret_type)`
- Defined: `lazyc.c:2575`

### parse_enum (function) `static void parse_enum(void)`
- Defined: `lazyc.c:2695`

### skip_struct (function) `static void skip_struct(void)`
- Defined: `lazyc.c:2732`

### skip_typedef (function) `static void skip_typedef(void)`
- Defined: `lazyc.c:2786`

### parse_program (function) `static void parse_program(void)`
- Defined: `lazyc.c:2824`

### emit_string_pool (function) `static void emit_string_pool(void)`
- Defined: `lazyc.c:2953`

### main (function) `int main(int argc, char **argv)`
- Defined: `lazyc.c:2977`
- Doc: Main entry point – can act as compiler or hotswap loader.

### va_start (function) `va_start(args, fmt);`
- Defined: `lazyc.c:181`

### va_end (function) `va_end(args);`
- Defined: `lazyc.c:185`

### memcpy (function) `memcpy(g_beacon_output + g_output_len, data, (size_t)len);`
- Defined: `lazyc.c:200`

### munmap (function) `munmap(code, code_size);`
- Defined: `lazyc.c:228`

### free (function) `free(g_trampolines);`
- Defined: `lazyc.c:245`

### volatile (function) `__asm__ volatile ( "push %%rbp\n\t" "mov %%rsp, %%rbp\n\t" "push %%rbx\n\t" "push %%r12\n\t" "push %%r13\n\t" "push %%r14\n\t" "push %%r15\n\t" "sub $8, %%rsp\n\t" "mov %0, %%rdi\n\t" "mov %1, %%rsi\n`
- Defined: `lazyc.c:304`

### fprintf (function) `fprintf(stderr, "[ERROR] dlsym failed for %s: %s\n", g_external_symbols[i].name, dlerror());`
- Defined: `lazyc.c:341`

### memset (function) `memset(addr, 0, aligned);`
- Defined: `lazyc.c:471`

### perror (function) `perror("open");`
- Defined: `lazyc.c:618`

### close (function) `close(fd);`
- Defined: `lazyc.c:625`

### fwrite (function) `fwrite(g_beacon_output, 1, g_output_len, stdout);`
- Defined: `lazyc.c:650`

### fputc (function) `fputc('\n', stdout);`
- Defined: `lazyc.c:651`

### exit (function) `exit(EXIT_FAILURE);`
- Defined: `lazyc.c:848`

### snprintf (function) `snprintf(token, MAX_TOKEN_LEN, "%d", macros[mi].value);`
- Defined: `lazyc.c:1046`

### strcpy (function) `strcpy(id_name, token);`
- Defined: `lazyc.c:1288`

### fseek (function) `fseek(input, 0, SEEK_END);`
- Defined: `lazyc.c:3000`

### fclose (function) `fclose(input);`
- Defined: `lazyc.c:3005`

### rewind (function) `rewind(input);`
- Defined: `lazyc.c:3008`

## module_hot.c

### go (function) `void go(char *args, int arglen)`
- Defined: `module_hot.c:2`
- Doc: #include <stdio.h>   /* printf, fflush

### printf (function) `printf("[+] Module loaded.\n");`
- Defined: `module_hot.c:4`

### fflush (function) `fflush(stdout);`
- Defined: `module_hot.c:6`
