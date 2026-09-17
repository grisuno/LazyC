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
- Defined: `lazyc.c:193`

### create_trampoline (function) `static void *create_trampoline(void *target)`
- Defined: `lazyc.c:209`
- Doc: Trampoline creation and management.

### cleanup_trampolines (function) `static void cleanup_trampolines(void)`
- Defined: `lazyc.c:241`

### get_or_create_trampoline (function) `static void *get_or_create_trampoline(void *target)`
- Defined: `lazyc.c:256`

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
- Defined: `lazyc.c:851`

### my_isspace (function) `static int my_isspace(int c)`
- Defined: `lazyc.c:860`

### my_isalpha (function) `static int my_isalpha(int c)`
- Defined: `lazyc.c:865`

### my_isdigit (function) `static int my_isdigit(int c)`
- Defined: `lazyc.c:869`

### my_isalnum (function) `static int my_isalnum(int c)`
- Defined: `lazyc.c:873`

### find_macro (function) `static int find_macro(const char *name)`
- Defined: `lazyc.c:877`

### add_macro (function) `static void add_macro(const char *name, int value)`
- Defined: `lazyc.c:885`

### save_parser_state (function) `static void save_parser_state(ParserState *state)`
- Defined: `lazyc.c:896`

### restore_parser_state (function) `static void restore_parser_state(ParserState *state)`
- Defined: `lazyc.c:920`

### next_token (function) `static void next_token(void)`
- Defined: `lazyc.c:944`

### match (function) `static void match(int expected)`
- Defined: `lazyc.c:1169`

### emit (function) `static void emit(const char *s)`
- Defined: `lazyc.c:1174`

### emit_i (function) `static void emit_i(const char *fmt, int v)`
- Defined: `lazyc.c:1188`

### emit_s (function) `static void emit_s(const char *fmt, const char *s)`
- Defined: `lazyc.c:1194`

### emit_is (function) `static void emit_is(const char *fmt, int v, const char *s)`
- Defined: `lazyc.c:1200`

### emit_si (function) `static void emit_si(const char *fmt, const char *s, int v)`
- Defined: `lazyc.c:1206`

### emit_label (function) `static void emit_label(int label)`
- Defined: `lazyc.c:1212`

### find_symbol (function) `static int find_symbol(const char *name)`
- Defined: `lazyc.c:1217`

### add_symbol (function) `static void add_symbol(const char *name, int is_global, int size, int pointed,
                  ...`
- Defined: `lazyc.c:1225`

### arg_reg (function) `static const char *arg_reg(int i)`
- Defined: `lazyc.c:1259`

### libc_global_name (function) `static const char *libc_global_name(int i)`
- Defined: `lazyc.c:1268`

### unary (function) `static void unary(void)`
- Defined: `lazyc.c:1281`

### lvalue_address (function) `static void lvalue_address(void)`
- Defined: `lazyc.c:1435`

### handle_postfix (function) `static void handle_postfix(int is_lvalue)`
- Defined: `lazyc.c:1480`

### unary_expr (function) `static void unary_expr(void)`
- Defined: `lazyc.c:1581`

### multiplicative_expr (function) `static void multiplicative_expr(void)`
- Defined: `lazyc.c:1586`

### additive_expr (function) `static void additive_expr(void)`
- Defined: `lazyc.c:1612`

### relational_expr (function) `static void relational_expr(void)`
- Defined: `lazyc.c:1630`

### equality_expr (function) `static void equality_expr(void)`
- Defined: `lazyc.c:1648`

### bitwise_and_expr (function) `static void bitwise_and_expr(void)`
- Defined: `lazyc.c:1664`

### bitwise_xor_expr (function) `static void bitwise_xor_expr(void)`
- Defined: `lazyc.c:1676`

### bitwise_or_expr (function) `static void bitwise_or_expr(void)`
- Defined: `lazyc.c:1688`

### logical_and_expr (function) `static void logical_and_expr(void)`
- Defined: `lazyc.c:1700`

### logical_or_expr (function) `static void logical_or_expr(void)`
- Defined: `lazyc.c:1720`

### conditional_expr (function) `static void conditional_expr(void)`
- Defined: `lazyc.c:1740`

### assignment_expr (function) `static void assignment_expr(void)`
- Defined: `lazyc.c:1758`

### statement (function) `static void statement(void)`
- Defined: `lazyc.c:1992`

### parse_function (function) `static void parse_function(const char *name, int ret_type)`
- Defined: `lazyc.c:2576`

### parse_enum (function) `static void parse_enum(void)`
- Defined: `lazyc.c:2696`

### skip_struct (function) `static void skip_struct(void)`
- Defined: `lazyc.c:2733`

### skip_typedef (function) `static void skip_typedef(void)`
- Defined: `lazyc.c:2787`

### parse_program (function) `static void parse_program(void)`
- Defined: `lazyc.c:2825`

### emit_string_pool (function) `static void emit_string_pool(void)`
- Defined: `lazyc.c:2954`

### main (function) `int main(int argc, char **argv)`
- Defined: `lazyc.c:2977`
- Doc: Main entry point – can act as compiler or hotswap loader.

## module_hot.c

### go (function) `void go(char *args, int arglen)`
- Defined: `module_hot.c:3`
