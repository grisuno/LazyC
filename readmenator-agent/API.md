# API

## lazyc.c

### BeaconPrintf `static void BeaconPrintf(int type, const char *fmt, ...)`
- Defined: `lazyc.c:177`
- Doc: Beacon API – capture output into global buffer.

### BeaconOutput `static void BeaconOutput(int type, const char *data, int len)`
- Defined: `lazyc.c:192`

### create_trampoline `static void *create_trampoline(void *target)`
- Defined: `lazyc.c:209`
- Doc: Trampoline creation and management.

### cleanup_trampolines `static void cleanup_trampolines(void)`
- Defined: `lazyc.c:240`

### get_or_create_trampoline `static void *get_or_create_trampoline(void *target)`
- Defined: `lazyc.c:255`

### page_align `static size_t page_align(size_t size)`
- Defined: `lazyc.c:291`
- Doc: Page alignment utility.

### call_bof_isolated `static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t arglen)`
- Defined: `lazyc.c:303`
- Doc: Isolated call to BOF function with correct stack alignment.

### resolve_external_symbols `static int resolve_external_symbols(void)`
- Defined: `lazyc.c:335`
- Doc: Resolve all external symbols needed by the BOF.

### run_elf `static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t filesize,
        ...`
- Defined: `lazyc.c:353`
- Doc: Core ELF loader: maps sections, resolves relocations, executes entry point.

### run_elf_from_file `static int run_elf_from_file(const char *filename, const char *funcname,
                        ...`
- Defined: `lazyc.c:612`
- Doc: Convenience wrapper: load ELF from file and run.

### error `static void error(const char *msg)`
- Defined: `lazyc.c:846`
- Doc: Error handling.

### safe_malloc `static void *safe_malloc(size_t size)`
- Defined: `lazyc.c:850`

### my_isspace `static int my_isspace(int c)`
- Defined: `lazyc.c:859`

### my_isalpha `static int my_isalpha(int c)`
- Defined: `lazyc.c:864`

### my_isdigit `static int my_isdigit(int c)`
- Defined: `lazyc.c:868`

### my_isalnum `static int my_isalnum(int c)`
- Defined: `lazyc.c:872`

### find_macro `static int find_macro(const char *name)`
- Defined: `lazyc.c:876`

### add_macro `static void add_macro(const char *name, int value)`
- Defined: `lazyc.c:884`

### save_parser_state `static void save_parser_state(ParserState *state)`
- Defined: `lazyc.c:895`

### restore_parser_state `static void restore_parser_state(ParserState *state)`
- Defined: `lazyc.c:919`

### next_token `static void next_token(void)`
- Defined: `lazyc.c:943`

### match `static void match(int expected)`
- Defined: `lazyc.c:1168`

### emit `static void emit(const char *s)`
- Defined: `lazyc.c:1173`

### emit_i `static void emit_i(const char *fmt, int v)`
- Defined: `lazyc.c:1187`

### emit_s `static void emit_s(const char *fmt, const char *s)`
- Defined: `lazyc.c:1193`

### emit_is `static void emit_is(const char *fmt, int v, const char *s)`
- Defined: `lazyc.c:1199`

### emit_si `static void emit_si(const char *fmt, const char *s, int v)`
- Defined: `lazyc.c:1205`

### emit_label `static void emit_label(int label)`
- Defined: `lazyc.c:1211`

### find_symbol `static int find_symbol(const char *name)`
- Defined: `lazyc.c:1216`

### add_symbol `static void add_symbol(const char *name, int is_global, int size, int pointed,
                  ...`
- Defined: `lazyc.c:1224`

### arg_reg `static const char *arg_reg(int i)`
- Defined: `lazyc.c:1258`

### libc_global_name `static const char *libc_global_name(int i)`
- Defined: `lazyc.c:1267`

### unary `static void unary(void)`
- Defined: `lazyc.c:1280`

### lvalue_address `static void lvalue_address(void)`
- Defined: `lazyc.c:1434`

### handle_postfix `static void handle_postfix(int is_lvalue)`
- Defined: `lazyc.c:1479`

### unary_expr `static void unary_expr(void)`
- Defined: `lazyc.c:1580`

### multiplicative_expr `static void multiplicative_expr(void)`
- Defined: `lazyc.c:1585`

### additive_expr `static void additive_expr(void)`
- Defined: `lazyc.c:1611`

### relational_expr `static void relational_expr(void)`
- Defined: `lazyc.c:1629`

### equality_expr `static void equality_expr(void)`
- Defined: `lazyc.c:1647`

### bitwise_and_expr `static void bitwise_and_expr(void)`
- Defined: `lazyc.c:1663`

### bitwise_xor_expr `static void bitwise_xor_expr(void)`
- Defined: `lazyc.c:1675`

### bitwise_or_expr `static void bitwise_or_expr(void)`
- Defined: `lazyc.c:1687`

### logical_and_expr `static void logical_and_expr(void)`
- Defined: `lazyc.c:1699`

### logical_or_expr `static void logical_or_expr(void)`
- Defined: `lazyc.c:1719`

### conditional_expr `static void conditional_expr(void)`
- Defined: `lazyc.c:1739`

### assignment_expr `static void assignment_expr(void)`
- Defined: `lazyc.c:1757`

### statement `static void statement(void)`
- Defined: `lazyc.c:1991`

### parse_function `static void parse_function(const char *name, int ret_type)`
- Defined: `lazyc.c:2575`

### parse_enum `static void parse_enum(void)`
- Defined: `lazyc.c:2695`

### skip_struct `static void skip_struct(void)`
- Defined: `lazyc.c:2732`

### skip_typedef `static void skip_typedef(void)`
- Defined: `lazyc.c:2786`

### parse_program `static void parse_program(void)`
- Defined: `lazyc.c:2824`

### emit_string_pool `static void emit_string_pool(void)`
- Defined: `lazyc.c:2953`

### main `int main(int argc, char **argv)`
- Defined: `lazyc.c:2977`
- Doc: Main entry point – can act as compiler or hotswap loader.

## module_hot.c

### go `void go(char *args, int arglen)`
- Defined: `module_hot.c:2`
- Doc: #include <stdio.h>   /* printf, fflush
