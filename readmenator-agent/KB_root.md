# Subsystem: root

## app.py
- Layer: utility
- Doc: app.py  Autor: Gris Iscomeback Correo electrónico: grisiscomeback[at]gmail[dot]com Fecha de creación: xx/xx/xxxx Licenci
- Language: py

## beacon.h
- Layer: utility
- Doc: beacon_api.h   Tipos de callback  Estructura para parsing de datos (opcional, para comandos complejos)
- Language: h
- Symbols:
  - `datap` (struct, line 14)
  - `BeaconDataParse` (function, line 21) `void BeaconDataParse(datap *parser, char *buffer, int size);`
  - `BeaconDataPtr` (function, line 22) `char *BeaconDataPtr(datap *parser, int size);`
  - `BeaconDataInt` (function, line 23) `int BeaconDataInt(datap *parser);`
  - `BeaconDataShort` (function, line 24) `short BeaconDataShort(datap *parser);`
  - `BeaconDataLength` (function, line 25) `int BeaconDataLength(datap *parser);`
  - `BeaconDataExtract` (function, line 26) `char *BeaconDataExtract(datap *parser, int *size);`
  - `BeaconPrintf` (function, line 27) `void BeaconPrintf(int type, const char *fmt, ...);`
  - `BeaconOutput` (function, line 28) `void BeaconOutput(int type, const char *data, int len);`
  - `BEACON_API_H` (macro, line 3) `#define BEACON_API_H`
  - `CALLBACK_OUTPUT` (macro, line 9) `#define CALLBACK_OUTPUT`
  - `CALLBACK_ERROR` (macro, line 10) `#define CALLBACK_ERROR`
  - `CALLBACK_OUTPUT_OEM` (macro, line 11) `#define CALLBACK_OUTPUT_OEM`

## install.sh
- Layer: utility
- Language: sh

## lazyc.c
- Layer: utility
- Language: c
- Symbols:
  - `hotswap_config` (struct, line 36)
  - `SymbolResolver` (struct, line 105)
  - `Trampoline` (struct, line 141)
  - `TrampolineCache` (struct, line 146)
  - `Symbol` (struct, line 714)
  - `Macro` (struct, line 769)
  - `ParserState` (struct, line 775)
  - `BeaconPrintf` (function, line 177) `static void BeaconPrintf(int type, const char *fmt, ...)`
  - `BeaconOutput` (function, line 193) `static void BeaconOutput(int type, const char *data, int len)`
  - `create_trampoline` (function, line 209) `static void *create_trampoline(void *target)`
  - `cleanup_trampolines` (function, line 241) `static void cleanup_trampolines(void)`
  - `get_or_create_trampoline` (function, line 256) `static void *get_or_create_trampoline(void *target)`
  - `page_align` (function, line 291) `static size_t page_align(size_t size)`
  - `call_bof_isolated` (function, line 303) `static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t arglen)`
  - `resolve_external_symbols` (function, line 335) `static int resolve_external_symbols(void)`
  - `run_elf` (function, line 353) `static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t filesize,
        ...`
  - `run_elf_from_file` (function, line 612) `static int run_elf_from_file(const char *filename, const char *funcname,
                        ...`
  - `error` (function, line 846) `static void error(const char *msg)`
  - `safe_malloc` (function, line 851) `static void *safe_malloc(size_t size)`
  - `my_isspace` (function, line 860) `static int my_isspace(int c)`
  - `my_isalpha` (function, line 865) `static int my_isalpha(int c)`
  - `my_isdigit` (function, line 869) `static int my_isdigit(int c)`
  - `my_isalnum` (function, line 873) `static int my_isalnum(int c)`
  - `find_macro` (function, line 877) `static int find_macro(const char *name)`
  - `add_macro` (function, line 885) `static void add_macro(const char *name, int value)`
  - `save_parser_state` (function, line 896) `static void save_parser_state(ParserState *state)`
  - `restore_parser_state` (function, line 920) `static void restore_parser_state(ParserState *state)`
  - `next_token` (function, line 944) `static void next_token(void)`
  - `match` (function, line 1169) `static void match(int expected)`
  - `emit` (function, line 1174) `static void emit(const char *s)`
  - `emit_i` (function, line 1188) `static void emit_i(const char *fmt, int v)`
  - `emit_s` (function, line 1194) `static void emit_s(const char *fmt, const char *s)`
  - `emit_is` (function, line 1200) `static void emit_is(const char *fmt, int v, const char *s)`
  - `emit_si` (function, line 1206) `static void emit_si(const char *fmt, const char *s, int v)`
  - `emit_label` (function, line 1212) `static void emit_label(int label)`
  - `find_symbol` (function, line 1217) `static int find_symbol(const char *name)`
  - `add_symbol` (function, line 1225) `static void add_symbol(const char *name, int is_global, int size, int pointed,
                  ...`
  - `arg_reg` (function, line 1259) `static const char *arg_reg(int i)`
  - `libc_global_name` (function, line 1268) `static const char *libc_global_name(int i)`
  - `unary` (function, line 1281) `static void unary(void)`
  - `lvalue_address` (function, line 1435) `static void lvalue_address(void)`
  - `handle_postfix` (function, line 1480) `static void handle_postfix(int is_lvalue)`
  - `unary_expr` (function, line 1581) `static void unary_expr(void)`
  - `multiplicative_expr` (function, line 1586) `static void multiplicative_expr(void)`
  - `additive_expr` (function, line 1612) `static void additive_expr(void)`
  - `relational_expr` (function, line 1630) `static void relational_expr(void)`
  - `equality_expr` (function, line 1648) `static void equality_expr(void)`
  - `bitwise_and_expr` (function, line 1664) `static void bitwise_and_expr(void)`
  - `bitwise_xor_expr` (function, line 1676) `static void bitwise_xor_expr(void)`
  - `bitwise_or_expr` (function, line 1688) `static void bitwise_or_expr(void)`
  - `logical_and_expr` (function, line 1700) `static void logical_and_expr(void)`
  - `logical_or_expr` (function, line 1720) `static void logical_or_expr(void)`
  - `conditional_expr` (function, line 1740) `static void conditional_expr(void)`
  - `assignment_expr` (function, line 1758) `static void assignment_expr(void)`
  - `statement` (function, line 1992) `static void statement(void)`
  - `parse_function` (function, line 2576) `static void parse_function(const char *name, int ret_type)`
  - `parse_enum` (function, line 2696) `static void parse_enum(void)`
  - `skip_struct` (function, line 2733) `static void skip_struct(void)`
  - `skip_typedef` (function, line 2787) `static void skip_typedef(void)`
  - `parse_program` (function, line 2825) `static void parse_program(void)`
  - `emit_string_pool` (function, line 2954) `static void emit_string_pool(void)`
  - `main` (function, line 2977) `int main(int argc, char **argv)`
  - `MAX_TOKEN_LEN` (macro, line 664) `#define MAX_TOKEN_LEN`
  - `MAX_SYMBOLS` (macro, line 665) `#define MAX_SYMBOLS`
  - `MAX_IDENT_LEN` (macro, line 666) `#define MAX_IDENT_LEN`
  - `MAX_SOURCE_SIZE` (macro, line 667) `#define MAX_SOURCE_SIZE`
  - `STACK_ALIGN` (macro, line 668) `#define STACK_ALIGN`
  - `MAX_CASES_PER_SWITCH` (macro, line 740) `#define MAX_CASES_PER_SWITCH`
  - `MAX_STRINGS` (macro, line 753) `#define MAX_STRINGS`
  - `MAX_STRUCT_MEMBERS` (macro, line 757) `#define MAX_STRUCT_MEMBERS`
  - `MAX_MACROS` (macro, line 767) `#define MAX_MACROS`

## lazyc.s
- Layer: utility
- Language: s

## lazyc2.s
- Layer: utility
- Language: s

## module_hot.c
- Layer: utility
- Language: c
- Symbols:
  - `go` (function, line 3) `void go(char *args, int arglen)`
