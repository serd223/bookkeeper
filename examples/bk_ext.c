#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "time.h"

#include "../gen/bk_ext.h"

void gen_example_prelude(String* book_buf) {
    print_string(book_buf, "// Put important definitions/declarations here\n");
}

size_t gen_example_dump_decl(String* book_buf, CCompound* ty, const char* dst_type) {
    print_string(book_buf, "void dump_example_%s(%s* item, %s dst);\n", ty->name, ty->name, dst_type);
    return 1;
}
size_t gen_example_parse_decl(String* book_buf, CCompound* ty) {
    print_string(book_buf, "int parse_example_%s(char* src, unsigned long len, %s* item);\n", ty->name, ty->name);
    return 1;
}

void gen_example_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro) {
    print_string(book_buf, "void dump_example_%s(%s* item, %s dst) {\n", ty->name, ty->name, dst_type);
    print_string(book_buf, "    %s(\"Example schema extension\\n\");\n", fmt_macro);
    print_string(book_buf, "}\n");
}
void gen_example_parse_impl(String* book_buf, CCompound* ty) {
    print_string(book_buf, "int parse_example_%s(char* src, unsigned long len, %s* item) {\n;", ty->name, ty->name);
    print_string(book_buf, "    return 0;\n");
    print_string(book_buf, "}\n");
}

// The `name` field should be unique among other schemas for
// macros that mention schemas to work properly
static StaticSchema example = {
    .gen_prelude = gen_example_prelude,
    .gen_dump_decl = gen_example_dump_decl,
    .gen_parse_decl = gen_example_parse_decl,
    .gen_dump_impl = gen_example_dump_impl,
    .gen_parse_impl = gen_example_parse_impl,
    .derive_attr = "derive_example",
    .name = "example"
};
#define BK_ADD_SCHEMAS(s)\
push_da(&s, example);

#include "../bk.c"
