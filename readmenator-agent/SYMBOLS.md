# Symbols

| Symbol | Kind | File:Line | Signature |
|--------|------|-----------|-----------|
| `BEACON_API_H` | macro | `beacon.h:3` | `#define BEACON_API_H` |
| `BeaconDataExtract` | function | `beacon.h:26` | `char *BeaconDataExtract(datap *parser, int *size);` |
| `BeaconDataInt` | function | `beacon.h:23` | `int BeaconDataInt(datap *parser);` |
| `BeaconDataLength` | function | `beacon.h:25` | `int BeaconDataLength(datap *parser);` |
| `BeaconDataParse` | function | `beacon.h:21` | `void BeaconDataParse(datap *parser, char *buffer, int size);` |
| `BeaconDataPtr` | function | `beacon.h:22` | `char *BeaconDataPtr(datap *parser, int size);` |
| `BeaconDataShort` | function | `beacon.h:24` | `short BeaconDataShort(datap *parser);` |
| `BeaconOutput` | function | `beacon.h:28` | `void BeaconOutput(int type, const char *data, int len);` |
| `BeaconPrintf` | function | `beacon.h:27` | `void BeaconPrintf(int type, const char *fmt, ...);` |
| `CALLBACK_ERROR` | macro | `beacon.h:10` | `#define CALLBACK_ERROR` |
| `CALLBACK_OUTPUT` | macro | `beacon.h:9` | `#define CALLBACK_OUTPUT` |
| `CALLBACK_OUTPUT_OEM` | macro | `beacon.h:11` | `#define CALLBACK_OUTPUT_OEM` |
| `datap` | struct | `beacon.h:14` | `` |
| `BeaconOutput` | function | `lazyc.c:193` | `static void BeaconOutput(int type, const char *data, int len)` |
| `BeaconPrintf` | function | `lazyc.c:177` | `static void BeaconPrintf(int type, const char *fmt, ...)` |
| `MAX_CASES_PER_SWITCH` | macro | `lazyc.c:740` | `#define MAX_CASES_PER_SWITCH` |
| `MAX_IDENT_LEN` | macro | `lazyc.c:666` | `#define MAX_IDENT_LEN` |
| `MAX_MACROS` | macro | `lazyc.c:767` | `#define MAX_MACROS` |
| `MAX_SOURCE_SIZE` | macro | `lazyc.c:667` | `#define MAX_SOURCE_SIZE` |
| `MAX_STRINGS` | macro | `lazyc.c:753` | `#define MAX_STRINGS` |
| `MAX_STRUCT_MEMBERS` | macro | `lazyc.c:757` | `#define MAX_STRUCT_MEMBERS` |
| `MAX_SYMBOLS` | macro | `lazyc.c:665` | `#define MAX_SYMBOLS` |
| `MAX_TOKEN_LEN` | macro | `lazyc.c:664` | `#define MAX_TOKEN_LEN` |
| `Macro` | struct | `lazyc.c:769` | `` |
| `ParserState` | struct | `lazyc.c:775` | `` |
| `STACK_ALIGN` | macro | `lazyc.c:668` | `#define STACK_ALIGN` |
| `Symbol` | struct | `lazyc.c:714` | `` |
| `SymbolResolver` | struct | `lazyc.c:105` | `` |
| `Trampoline` | struct | `lazyc.c:141` | `` |
| `TrampolineCache` | struct | `lazyc.c:146` | `` |
| `add_macro` | function | `lazyc.c:885` | `static void add_macro(const char *name, int value)` |
| `add_symbol` | function | `lazyc.c:1225` | `static void add_symbol(const char *name, int is_global, int size, int pointed,                   ...` |
| `additive_expr` | function | `lazyc.c:1612` | `static void additive_expr(void)` |
| `arg_reg` | function | `lazyc.c:1259` | `static const char *arg_reg(int i)` |
| `assignment_expr` | function | `lazyc.c:1758` | `static void assignment_expr(void)` |
| `bitwise_and_expr` | function | `lazyc.c:1664` | `static void bitwise_and_expr(void)` |
| `bitwise_or_expr` | function | `lazyc.c:1688` | `static void bitwise_or_expr(void)` |
| `bitwise_xor_expr` | function | `lazyc.c:1676` | `static void bitwise_xor_expr(void)` |
| `call_bof_isolated` | function | `lazyc.c:303` | `static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t arglen)` |
| `cleanup_trampolines` | function | `lazyc.c:241` | `static void cleanup_trampolines(void)` |
| `conditional_expr` | function | `lazyc.c:1740` | `static void conditional_expr(void)` |
| `create_trampoline` | function | `lazyc.c:209` | `static void *create_trampoline(void *target)` |
| `emit` | function | `lazyc.c:1174` | `static void emit(const char *s)` |
| `emit_i` | function | `lazyc.c:1188` | `static void emit_i(const char *fmt, int v)` |
| `emit_is` | function | `lazyc.c:1200` | `static void emit_is(const char *fmt, int v, const char *s)` |
| `emit_label` | function | `lazyc.c:1212` | `static void emit_label(int label)` |
| `emit_s` | function | `lazyc.c:1194` | `static void emit_s(const char *fmt, const char *s)` |
| `emit_si` | function | `lazyc.c:1206` | `static void emit_si(const char *fmt, const char *s, int v)` |
| `emit_string_pool` | function | `lazyc.c:2954` | `static void emit_string_pool(void)` |
| `equality_expr` | function | `lazyc.c:1648` | `static void equality_expr(void)` |
| `error` | function | `lazyc.c:846` | `static void error(const char *msg)` |
| `find_macro` | function | `lazyc.c:877` | `static int find_macro(const char *name)` |
| `find_symbol` | function | `lazyc.c:1217` | `static int find_symbol(const char *name)` |
| `get_or_create_trampoline` | function | `lazyc.c:256` | `static void *get_or_create_trampoline(void *target)` |
| `handle_postfix` | function | `lazyc.c:1480` | `static void handle_postfix(int is_lvalue)` |
| `hotswap_config` | struct | `lazyc.c:36` | `` |
| `libc_global_name` | function | `lazyc.c:1268` | `static const char *libc_global_name(int i)` |
| `logical_and_expr` | function | `lazyc.c:1700` | `static void logical_and_expr(void)` |
| `logical_or_expr` | function | `lazyc.c:1720` | `static void logical_or_expr(void)` |
| `lvalue_address` | function | `lazyc.c:1435` | `static void lvalue_address(void)` |
| `main` | function | `lazyc.c:2977` | `int main(int argc, char **argv)` |
| `match` | function | `lazyc.c:1169` | `static void match(int expected)` |
| `multiplicative_expr` | function | `lazyc.c:1586` | `static void multiplicative_expr(void)` |
| `my_isalnum` | function | `lazyc.c:873` | `static int my_isalnum(int c)` |
| `my_isalpha` | function | `lazyc.c:865` | `static int my_isalpha(int c)` |
| `my_isdigit` | function | `lazyc.c:869` | `static int my_isdigit(int c)` |
| `my_isspace` | function | `lazyc.c:860` | `static int my_isspace(int c)` |
| `next_token` | function | `lazyc.c:944` | `static void next_token(void)` |
| `page_align` | function | `lazyc.c:291` | `static size_t page_align(size_t size)` |
| `parse_enum` | function | `lazyc.c:2696` | `static void parse_enum(void)` |
| `parse_function` | function | `lazyc.c:2576` | `static void parse_function(const char *name, int ret_type)` |
| `parse_program` | function | `lazyc.c:2825` | `static void parse_program(void)` |
| `relational_expr` | function | `lazyc.c:1630` | `static void relational_expr(void)` |
| `resolve_external_symbols` | function | `lazyc.c:335` | `static int resolve_external_symbols(void)` |
| `restore_parser_state` | function | `lazyc.c:920` | `static void restore_parser_state(ParserState *state)` |
| `run_elf` | function | `lazyc.c:353` | `static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t filesize,         ...` |
| `run_elf_from_file` | function | `lazyc.c:612` | `static int run_elf_from_file(const char *filename, const char *funcname,                         ...` |
| `safe_malloc` | function | `lazyc.c:851` | `static void *safe_malloc(size_t size)` |
| `save_parser_state` | function | `lazyc.c:896` | `static void save_parser_state(ParserState *state)` |
| `skip_struct` | function | `lazyc.c:2733` | `static void skip_struct(void)` |
| `skip_typedef` | function | `lazyc.c:2787` | `static void skip_typedef(void)` |
| `statement` | function | `lazyc.c:1992` | `static void statement(void)` |
| `unary` | function | `lazyc.c:1281` | `static void unary(void)` |
| `unary_expr` | function | `lazyc.c:1581` | `static void unary_expr(void)` |
| `go` | function | `module_hot.c:3` | `void go(char *args, int arglen)` |
