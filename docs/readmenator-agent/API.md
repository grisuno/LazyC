# API

## beacon.h
- `BeaconDataParse` (function) `beacon.h:21` `void BeaconDataParse(datap *parser, char *buffer, int size);` -- === API para BOFs ===
- `BeaconDataPtr` (function) `beacon.h:22` `char *BeaconDataPtr(datap *parser, int size);`
- `BeaconDataInt` (function) `beacon.h:23` `int BeaconDataInt(datap *parser);`
- `BeaconDataShort` (function) `beacon.h:24` `short BeaconDataShort(datap *parser);`
- `BeaconDataLength` (function) `beacon.h:25` `int BeaconDataLength(datap *parser);`
- `BeaconDataExtract` (function) `beacon.h:26` `char *BeaconDataExtract(datap *parser, int *size);`
- `BeaconPrintf` (function) `beacon.h:27` `void BeaconPrintf(int type, const char *fmt, ...);`
- `BeaconOutput` (function) `beacon.h:28` `void BeaconOutput(int type, const char *data, int len);`

## lazyc.c
- `BeaconPrintf` (function) `lazyc.c:177` `static void BeaconPrintf(int type, const char *fmt, ...)` -- Beacon API – capture output into global buffer.
- `BeaconOutput` (function) `lazyc.c:193` `static void BeaconOutput(int type, const char *data, int len)`
- `create_trampoline` (function) `lazyc.c:209` `static void *create_trampoline(void *target)` -- Trampoline creation and management.
- `cleanup_trampolines` (function) `lazyc.c:241` `static void cleanup_trampolines(void)`
- `get_or_create_trampoline` (function) `lazyc.c:256` `static void *get_or_create_trampoline(void *target)`
- `page_align` (function) `lazyc.c:291` `static size_t page_align(size_t size)` -- Page alignment utility.
- `call_bof_isolated` (function) `lazyc.c:303` `static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t arglen)` -- Isolated call to BOF function with correct stack alignment.
- `resolve_external_symbols` (function) `lazyc.c:335` `static int resolve_external_symbols(void)` -- Resolve all external symbols needed by the BOF.
- `run_elf` (function) `lazyc.c:353` `static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t filesize,
        ...` -- Core ELF loader: maps sections, resolves relocations, executes entry point.
- `run_elf_from_file` (function) `lazyc.c:612` `static int run_elf_from_file(const char *filename, const char *funcname,
                        ...` -- Convenience wrapper: load ELF from file and run.
- `error` (function) `lazyc.c:846` `static void error(const char *msg)` -- Error handling.
- `safe_malloc` (function) `lazyc.c:851` `static void *safe_malloc(size_t size)`
- `my_isspace` (function) `lazyc.c:860` `static int my_isspace(int c)`
- `my_isalpha` (function) `lazyc.c:865` `static int my_isalpha(int c)`
- `my_isdigit` (function) `lazyc.c:869` `static int my_isdigit(int c)`
- `my_isalnum` (function) `lazyc.c:873` `static int my_isalnum(int c)`
- `find_macro` (function) `lazyc.c:877` `static int find_macro(const char *name)`
- `add_macro` (function) `lazyc.c:885` `static void add_macro(const char *name, int value)`
- `save_parser_state` (function) `lazyc.c:896` `static void save_parser_state(ParserState *state)`
- `restore_parser_state` (function) `lazyc.c:920` `static void restore_parser_state(ParserState *state)`
- `next_token` (function) `lazyc.c:944` `static void next_token(void)`
- `match` (function) `lazyc.c:1169` `static void match(int expected)`
- `emit` (function) `lazyc.c:1174` `static void emit(const char *s)`
- `emit_i` (function) `lazyc.c:1188` `static void emit_i(const char *fmt, int v)`
- `emit_s` (function) `lazyc.c:1194` `static void emit_s(const char *fmt, const char *s)`
- `emit_is` (function) `lazyc.c:1200` `static void emit_is(const char *fmt, int v, const char *s)`
- `emit_si` (function) `lazyc.c:1206` `static void emit_si(const char *fmt, const char *s, int v)`
- `emit_label` (function) `lazyc.c:1212` `static void emit_label(int label)`
- `find_symbol` (function) `lazyc.c:1217` `static int find_symbol(const char *name)`
- `add_symbol` (function) `lazyc.c:1225` `static void add_symbol(const char *name, int is_global, int size, int pointed,
                  ...`
- `arg_reg` (function) `lazyc.c:1259` `static const char *arg_reg(int i)`
- `libc_global_name` (function) `lazyc.c:1268` `static const char *libc_global_name(int i)`
- `unary` (function) `lazyc.c:1281` `static void unary(void)`
- `lvalue_address` (function) `lazyc.c:1435` `static void lvalue_address(void)`
- `handle_postfix` (function) `lazyc.c:1480` `static void handle_postfix(int is_lvalue)`
- `unary_expr` (function) `lazyc.c:1581` `static void unary_expr(void)`
- `multiplicative_expr` (function) `lazyc.c:1586` `static void multiplicative_expr(void)`
- `additive_expr` (function) `lazyc.c:1612` `static void additive_expr(void)`
- `relational_expr` (function) `lazyc.c:1630` `static void relational_expr(void)`
- `equality_expr` (function) `lazyc.c:1648` `static void equality_expr(void)`
- `bitwise_and_expr` (function) `lazyc.c:1664` `static void bitwise_and_expr(void)`
- `bitwise_xor_expr` (function) `lazyc.c:1676` `static void bitwise_xor_expr(void)`
- `bitwise_or_expr` (function) `lazyc.c:1688` `static void bitwise_or_expr(void)`
- `logical_and_expr` (function) `lazyc.c:1700` `static void logical_and_expr(void)`
- `logical_or_expr` (function) `lazyc.c:1720` `static void logical_or_expr(void)`
- `conditional_expr` (function) `lazyc.c:1740` `static void conditional_expr(void)`
- `assignment_expr` (function) `lazyc.c:1758` `static void assignment_expr(void)`
- `statement` (function) `lazyc.c:1992` `static void statement(void)`
- `parse_function` (function) `lazyc.c:2576` `static void parse_function(const char *name, int ret_type)`
- `parse_enum` (function) `lazyc.c:2696` `static void parse_enum(void)`
- `skip_struct` (function) `lazyc.c:2733` `static void skip_struct(void)`
- `skip_typedef` (function) `lazyc.c:2787` `static void skip_typedef(void)`
- `parse_program` (function) `lazyc.c:2825` `static void parse_program(void)`
- `emit_string_pool` (function) `lazyc.c:2954` `static void emit_string_pool(void)`
- `main` (function) `lazyc.c:2977` `int main(int argc, char **argv)` -- Main entry point – can act as compiler or hotswap loader.

## module_hot.c
- `go` (function) `module_hot.c:3` `void go(char *args, int arglen)`
