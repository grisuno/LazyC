# root

*Community 0 | 7 files | cohesion 1.00*

## Definition

This community groups 7 file(s) rooted at `root` with dominant language c (cohesion 1.00). Central symbols: `BEACON_API_H`, `BeaconDataExtract`, `BeaconDataInt`, `BeaconDataLength`, `BeaconDataParse`, `BeaconDataPtr`, `BeaconDataShort`, `BeaconOutput`. Core file: `lazyc.c` (71 symbols). Documented purpose: Autor: Gris Iscomeback Correo electrónico: grisiscomeback[at]gmail[dot]com Fecha de creación: xx/xx/xxxx Licencia: GPL v3  Descripción:.

## Files

| File | Language | Layer | Symbols | Doc |
|------|----------|-------|---------|-----|
| `app.py` | py | utility | 0 | yes |
| `beacon.h` | h | utility | 13 | yes |
| `install.sh` | sh | utility | 0 | no |
| `lazyc.c` | c | utility | 71 | no |
| `lazyc.s` | s | utility | 0 | no |
| `lazyc2.s` | s | utility | 0 | no |
| `module_hot.c` | c | utility | 1 | no |

## Key Symbols

- `BEACON_API_H` (macro, `beacon.h:3`) `#define BEACON_API_H`
- `CALLBACK_OUTPUT` (macro, `beacon.h:9`) `#define CALLBACK_OUTPUT`
- `CALLBACK_ERROR` (macro, `beacon.h:10`) `#define CALLBACK_ERROR`
- `CALLBACK_OUTPUT_OEM` (macro, `beacon.h:11`) `#define CALLBACK_OUTPUT_OEM`
- `datap` (struct, `beacon.h:14`) - Estructura para parsing de datos (opcional, para comandos complejos)
- `BeaconDataParse` (function, `beacon.h:21`) `void BeaconDataParse(datap *parser, char *buffer, int size);` - === API para BOFs ===
- `BeaconDataPtr` (function, `beacon.h:22`) `char *BeaconDataPtr(datap *parser, int size);`
- `BeaconDataInt` (function, `beacon.h:23`) `int BeaconDataInt(datap *parser);`
- `BeaconDataShort` (function, `beacon.h:24`) `short BeaconDataShort(datap *parser);`
- `BeaconDataLength` (function, `beacon.h:25`) `int BeaconDataLength(datap *parser);`
- `BeaconDataExtract` (function, `beacon.h:26`) `char *BeaconDataExtract(datap *parser, int *size);`
- `BeaconPrintf` (function, `beacon.h:27`) `void BeaconPrintf(int type, const char *fmt, ...);`
- `BeaconOutput` (function, `beacon.h:28`) `void BeaconOutput(int type, const char *data, int len);`
- `hotswap_config` (struct, `lazyc.c:36`) - Configuration structure – centralises all tunable parameters.
- `SymbolResolver` (struct, `lazyc.c:105`) - Symbol resolver entry.
- `Trampoline` (struct, `lazyc.c:141`) - Trampoline structures for far calls.
- `TrampolineCache` (struct, `lazyc.c:146`)
- `BeaconPrintf` (function, `lazyc.c:177`) `static void BeaconPrintf(int type, const char *fmt, ...)` - Beacon API – capture output into global buffer.
- `BeaconOutput` (function, `lazyc.c:193`) `static void BeaconOutput(int type, const char *data, int len)`
- `create_trampoline` (function, `lazyc.c:209`) `static void *create_trampoline(void *target)` - Trampoline creation and management.
- `cleanup_trampolines` (function, `lazyc.c:241`) `static void cleanup_trampolines(void)`
- `get_or_create_trampoline` (function, `lazyc.c:256`) `static void *get_or_create_trampoline(void *target)`
- `page_align` (function, `lazyc.c:291`) `static size_t page_align(size_t size)` - Page alignment utility.
- `call_bof_isolated` (function, `lazyc.c:303`) `static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t a` - Isolated call to BOF function with correct stack alignment.
- `resolve_external_symbols` (function, `lazyc.c:335`) `static int resolve_external_symbols(void)` - Resolve all external symbols needed by the BOF.
- `run_elf` (function, `lazyc.c:353`) `static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t f` - Core ELF loader: maps sections, resolves relocations, executes entry point.
- `run_elf_from_file` (function, `lazyc.c:612`) `static int run_elf_from_file(const char *filename, const char *funcname,` - Convenience wrapper: load ELF from file and run.
- `MAX_TOKEN_LEN` (macro, `lazyc.c:664`) `#define MAX_TOKEN_LEN`
- `MAX_SYMBOLS` (macro, `lazyc.c:665`) `#define MAX_SYMBOLS`
- `MAX_IDENT_LEN` (macro, `lazyc.c:666`) `#define MAX_IDENT_LEN`

## Internal vs External Edges

- Internal resolved imports (EXTRACTED): 0
- Cross-boundary resolved imports (EXTRACTED): 0

## Connections

- No cross-community bridges recorded. This community is self-contained.

## Risks

- No scoped security, taint, cycle, or layer risks.

## Open Questions

- Why do 5 file(s) lack file-level docs (e.g. `install.sh`)? What purpose do they serve?
- What would break if the most connected file in root changed?
- Should root be split, given cohesion 1.00?

## Sources

- `app.py`
- `beacon.h`
- `install.sh`
- `lazyc.c`
- `lazyc.s`
- `lazyc2.s`
- `module_hot.c`
