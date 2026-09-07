# Subsystem: root

## app.py
- Layer: utility
- Doc: _*_ coding: utf8 _*_
- Language: py

## beacon.h
- Layer: utility
- Doc: beacon_api.h ifndef BEACON_API_H define BEACON_API_H  include <stdint.h> include <stdarg.h>  Tipos de callback define CA
- Language: h
- Symbols:
  - `BEACON_API_H` (macro, line 3)
  - `CALLBACK_OUTPUT` (macro, line 9)
  - `CALLBACK_ERROR` (macro, line 10)
  - `CALLBACK_OUTPUT_OEM` (macro, line 11)

## install.sh
- Layer: utility
- Language: sh

## lazyc.c
- Layer: utility
- Language: c
- Symbols:
  - `hotswap_config` (struct, line 36)
  - `BeaconPrintf` (function, line 177) `static void BeaconPrintf(int type, const char *fmt, ...)`
  - `BeaconOutput` (function, line 192) `static void BeaconOutput(int type, const char *data, int len)`
  - `create_trampoline` (function, line 209) `static void *create_trampoline(void *target)`
  - `cleanup_trampolines` (function, line 240) `static void cleanup_trampolines(void)`
  - `get_or_create_trampoline` (function, line 255) `static void *get_or_create_trampoline(void *target)`
  - `page_align` (function, line 291) `static size_t page_align(size_t size)`
  - `call_bof_isolated` (function, line 303) `static void call_bof_isolated(void (*func)(char *, int), char *args, uintptr_t arglen)`
  - `resolve_external_symbols` (function, line 335) `static int resolve_external_symbols(void)`
  - `run_elf` (function, line 353) `static int run_elf(const char *functionname, unsigned char *elf_data, uint32_t filesize,
        ...`
  - `run_elf_from_file` (function, line 612) `static int run_elf_from_file(const char *filename, const char *funcname,
                        ...`
  - `error` (function, line 846) `static void error(const char *msg)`
  - `safe_malloc` (function, line 850) `static void *safe_malloc(size_t size)`
  - `my_isspace` (function, line 859) `static int my_isspace(int c)`
  - `my_isalpha` (function, line 864) `static int my_isalpha(int c)`
  - `my_isdigit` (function, line 868) `static int my_isdigit(int c)`
  - `my_isalnum` (function, line 872) `static int my_isalnum(int c)`
  - `find_macro` (function, line 876) `static int find_macro(const char *name)`
  - `add_macro` (function, line 884) `static void add_macro(const char *name, int value)`
  - `save_parser_state` (function, line 895) `static void save_parser_state(ParserState *state)`
  - `restore_parser_state` (function, line 919) `static void restore_parser_state(ParserState *state)`
  - `next_token` (function, line 943) `static void next_token(void)`
  - `match` (function, line 1168) `static void match(int expected)`
  - `emit` (function, line 1173) `static void emit(const char *s)`
  - `emit_i` (function, line 1187) `static void emit_i(const char *fmt, int v)`
  - `emit_s` (function, line 1193) `static void emit_s(const char *fmt, const char *s)`
  - `emit_is` (function, line 1199) `static void emit_is(const char *fmt, int v, const char *s)`
  - `emit_si` (function, line 1205) `static void emit_si(const char *fmt, const char *s, int v)`
  - `emit_label` (function, line 1211) `static void emit_label(int label)`
  - `find_symbol` (function, line 1216) `static int find_symbol(const char *name)`
  - `add_symbol` (function, line 1224) `static void add_symbol(const char *name, int is_global, int size, int pointed,
                  ...`
  - `arg_reg` (function, line 1258) `static const char *arg_reg(int i)`
  - `libc_global_name` (function, line 1267) `static const char *libc_global_name(int i)`
  - `unary` (function, line 1280) `static void unary(void)`
  - `lvalue_address` (function, line 1434) `static void lvalue_address(void)`
  - `handle_postfix` (function, line 1479) `static void handle_postfix(int is_lvalue)`
  - `unary_expr` (function, line 1580) `static void unary_expr(void)`
  - `multiplicative_expr` (function, line 1585) `static void multiplicative_expr(void)`
  - `additive_expr` (function, line 1611) `static void additive_expr(void)`
  - `relational_expr` (function, line 1629) `static void relational_expr(void)`
  - `equality_expr` (function, line 1647) `static void equality_expr(void)`
  - `bitwise_and_expr` (function, line 1663) `static void bitwise_and_expr(void)`
  - `bitwise_xor_expr` (function, line 1675) `static void bitwise_xor_expr(void)`
  - `bitwise_or_expr` (function, line 1687) `static void bitwise_or_expr(void)`
  - `logical_and_expr` (function, line 1699) `static void logical_and_expr(void)`
  - `logical_or_expr` (function, line 1719) `static void logical_or_expr(void)`
  - `conditional_expr` (function, line 1739) `static void conditional_expr(void)`
  - `assignment_expr` (function, line 1757) `static void assignment_expr(void)`
  - `statement` (function, line 1991) `static void statement(void)`
  - `parse_function` (function, line 2575) `static void parse_function(const char *name, int ret_type)`
  - `parse_enum` (function, line 2695) `static void parse_enum(void)`
  - `skip_struct` (function, line 2732) `static void skip_struct(void)`
  - `skip_typedef` (function, line 2786) `static void skip_typedef(void)`
  - `parse_program` (function, line 2824) `static void parse_program(void)`
  - `emit_string_pool` (function, line 2953) `static void emit_string_pool(void)`
  - `main` (function, line 2977) `int main(int argc, char **argv)`
  - `MAX_TOKEN_LEN` (macro, line 663)
  - `MAX_SYMBOLS` (macro, line 665)
  - `MAX_IDENT_LEN` (macro, line 666)
  - `MAX_SOURCE_SIZE` (macro, line 667)
  - `STACK_ALIGN` (macro, line 668)
  - `MAX_CASES_PER_SWITCH` (macro, line 739)
  - `MAX_STRINGS` (macro, line 753)
  - `MAX_STRUCT_MEMBERS` (macro, line 756)
  - `MAX_MACROS` (macro, line 766)

## lazyc.s
- Layer: utility
- Language: s

## lazyc2.s
- Layer: utility
- Language: s

## module_hot.c
- Layer: utility
- Doc: include <stdio.h>   /* printf, fflush */
- Language: c
- Symbols:
  - `go` (function, line 2) `void go(char *args, int arglen)`
