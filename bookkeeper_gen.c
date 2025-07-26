#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"
#ifdef DEBUG
#define printf(...)
#endif

typedef struct {
    char* items;
    size_t len;
    size_t cap;
} String;

typedef enum {
    CPRIMITIVE,
    CEXTERNAL
} CType_Kind;

typedef enum {
    CINT = 1,
    CUINT,
    CLONG,
    CULONG,
    CCHAR,
    CFLOAT,
    CBOOL,
    CSTRING
} CPrimitive;

typedef struct {
    CType_Kind kind;
    const char* name; // CEXTERNAL
    CPrimitive type;  // CPIRIMITIVE
} CType;

typedef struct {
    CType* items;
    size_t len;
    size_t cap;
} CTypes;

typedef struct {
    const char* name;
    CType type;
} Field;

typedef struct {
    Field* items;
    size_t len;
    size_t cap;
} Fields;

#define DERIVE_JSON 0b1
#define DERIVE_ALL DERIVE_JSON

typedef struct {
    int derived_schemas;
    Fields fields;
    const char* name;
} CCompound;

typedef struct {
    CCompound* items;
    size_t len;
    size_t cap;
} CCompounds;

#define push_da(arr, item)                                                          \
    if ((arr)->len + 1 >= (arr)->cap) {                                             \
        if ((arr)->cap > 0) {                                                       \
            (arr)->cap = (arr)->cap * 2;                                            \
        } else {                                                                    \
            (arr)->cap = 2;                                                         \
        }                                                                           \
        if ((arr)->items) {                                                         \
            (arr)->items = realloc((arr)->items, (arr)->cap * sizeof *(arr)->items);\
        } else {                                                                    \
            (arr)->items = malloc((arr)->cap * sizeof *(arr)->items);               \
        }                                                                           \
    }                                                                               \
    (arr)->items[(arr)->len++] = (item);

#define print_string(str, ...) do {                                                 \
    int len = sprintf(tmp_str, __VA_ARGS__);                                        \
    if ((str)->cap <= ((unsigned long)len + (str)->len)) {                          \
        (str)->cap += len + 8 - (len % 8);                                          \
        if ((str)->items) {                                                         \
            (str)->items = realloc((str)->items, (str)->cap * sizeof *(str)->items);\
        } else {                                                                    \
            (str)->items = malloc((str)->cap * sizeof *(str)->items);               \
        }                                                                           \
    }                                                                               \
    memcpy((str)->items + (str)->len, tmp_str, len);                                \
    (str)->len += len;                                                              \
} while(0)

void read_entire_file(const char* file_name, String* out) {
    struct stat s = {0};
    stat(file_name, &s);
    size_t f_len = s.st_size / sizeof(char);
    if (f_len >= (out->cap - out->len)) {
        out->cap += f_len; // allocate extra space beacuse why not ram is cheap
        if (out->items) {
            out->items = realloc(out->items, out->cap);
        } else {
            out->items = malloc(out->cap);
        }
    }
    FILE* f = fopen(file_name, "rb");
    out->len += fread(out->items + out->len, sizeof(char), f_len, f) / sizeof(char);
}


bool va_get_expect_ids(stb_lexer* lex, int ids_count, va_list args) {
    for (size_t i = 0; i < ids_count; ++i) {
        stb_c_lexer_get_token(lex);
        const char* expected = va_arg(args, const char*);
        if (!(lex->token == CLEX_id))           return false;
        if (strcmp(lex->string, expected) != 0) return false;
    }
    return true;
}

bool va_get_expect_tokens(stb_lexer* lex, int ids_count, va_list args) {
    for (size_t i = 0; i < ids_count; ++i) {
        stb_c_lexer_get_token(lex);
        int expected = va_arg(args, int);
        if (!(lex->token == expected)) return false;
    }
    return true;
}

bool peek_ids(stb_lexer* lex, int ids_count, ...) {
    va_list args;
    va_start(args, ids_count);
    char* pp = lex->parse_point;
    bool res = va_get_expect_ids(lex, ids_count, args);
    lex->parse_point = pp;
    va_end(args);
    return res;
}

bool peek_tokens(stb_lexer* lex, int ids_count, ...) {
    va_list args;
    va_start(args, ids_count);
    char* pp = lex->parse_point;
    bool res = va_get_expect_tokens(lex, ids_count, args);
    lex->parse_point = pp;
    va_end(args);
    return res;
}

bool get_expect_tokens(stb_lexer* lex, int ids_count, ...) {
    va_list args;
    va_start(args, ids_count);
    bool res = va_get_expect_tokens(lex, ids_count, args);
    va_end(args);
    return res;
}

bool get_expect_ids(stb_lexer* lex, int ids_count, ...) {
    va_list args;
    va_start(args, ids_count);
    bool res = va_get_expect_ids(lex, ids_count, args);
    va_end(args);
    return res;
}

void analyze_file(String content, CCompounds* out, bool derive_all) {
    static char string_store[4096];
    stb_lexer lex = {0};
    stb_c_lexer_init(&lex, content.items, content.items + content.len, string_store, sizeof string_store);
    for (;;) {
        char* pp = lex.parse_point;
        stb_c_lexer_get_token(&lex);
        if (!lex.token || lex.token == CLEX_eof) break;
        lex.parse_point = pp;
        // NOTE: this tool can only understand typedef struct {} Name; style definitions
        if (get_expect_ids(&lex, 2, "typedef", "struct")) {
            if (!(get_expect_tokens(&lex, 1, '{'))) continue;
            CCompound strct = {0};
            if (derive_all) strct.derived_schemas |= DERIVE_ALL;
            for (;;) {
                Field field = {0};
                if (peek_tokens(&lex, 5, CLEX_id, CLEX_id, '*', CLEX_id, ';') && peek_ids(&lex, 2, "const", "char")) { 
                    stb_c_lexer_get_token(&lex); // const
                    stb_c_lexer_get_token(&lex); // char
                    stb_c_lexer_get_token(&lex); // *
                    stb_c_lexer_get_token(&lex); // name
                    field.name = strdup(lex.string); // leaks
                    stb_c_lexer_get_token(&lex); // ;
                    field.type.kind = CPRIMITIVE;
                    field.type.type = CSTRING;
                    push_da(&strct.fields, field);

                } else if (peek_tokens(&lex, 4, CLEX_id, '*', CLEX_id, ';') && peek_ids(&lex, 1, "char")) {
                    stb_c_lexer_get_token(&lex); // char
                    stb_c_lexer_get_token(&lex); // *
                    stb_c_lexer_get_token(&lex); // name
                    field.name = strdup(lex.string); // leaks
                    stb_c_lexer_get_token(&lex); // ;
                    field.type.kind = CPRIMITIVE;
                    field.type.type = CSTRING;
                    push_da(&strct.fields, field);
                } else if (peek_tokens(&lex, 4, CLEX_id, CLEX_id, CLEX_id, ';')) {
                    bool is_known_primitive = true;
                    if (peek_ids(&lex, 2, "unsigned", "int")) {
                        field.type.type = CUINT;
                    } else if (peek_ids(&lex, 2, "unsigned", "long")) {
                        field.type.type = CULONG;
                    } else {
                        is_known_primitive = false;
                    }
                    if (is_known_primitive) {
                        field.type.kind = CPRIMITIVE;
                        stb_c_lexer_get_token(&lex); // type name 1
                        stb_c_lexer_get_token(&lex); // type name 2
                        stb_c_lexer_get_token(&lex); // name
                        field.name = strdup(lex.string); // leaks
                        stb_c_lexer_get_token(&lex); // ;
                        push_da(&strct.fields, field);
                    } else {
                        // TODO: Report unknown field error
                        break;
                    }
                    
                } else if (peek_tokens(&lex, 3, CLEX_id, CLEX_id, ';')) {
                    bool is_known_primitive = true;
                    if (peek_ids(&lex, 1, "int")) {
                        field.type.type = CINT;
                    } else if (peek_ids(&lex, 1, "long")) {
                        field.type.type = CLONG;
                    } else if (peek_ids(&lex, 1, "size_t")) {
                        field.type.type = CULONG;
                    } else if (peek_ids(&lex, 1, "double") || peek_ids(&lex, 1, "float")) {
                        field.type.type = CFLOAT;
                    } else if (peek_ids(&lex, 1, "char")) {
                        field.type.type = CCHAR;
                    } else if (peek_ids(&lex, 1, "double") || peek_ids(&lex, 1, "float")) {
                        field.type.type = CFLOAT;
                    } else if (peek_ids(&lex, 1, "bool")) {
                        field.type.type = CBOOL;
                    } else {
                        is_known_primitive = false;
                    }

                    stb_c_lexer_get_token(&lex); // type name
                    if (!is_known_primitive) {
                        field.type.kind = CEXTERNAL;
                        field.type.name = strdup(lex.string); // leaks
                    } else {
                        field.type.kind = CPRIMITIVE;
                    }
                    stb_c_lexer_get_token(&lex); // name
                    field.name = strdup(lex.string); // leaks
                    stb_c_lexer_get_token(&lex); // ;
                    push_da(&strct.fields, field);
                } else {
                    // TODO: Report unknown field error
                    break;
                }
            }
            // `continue`ing here can leak unused field/type names of partially correct structs
            if (!get_expect_tokens(&lex, 2, '}', CLEX_id)) continue;
            strct.name = strdup(lex.string); // leaks (static data)
            if (peek_tokens(&lex, 3, CLEX_id, '(', ')') && peek_ids(&lex, 1, "derive_json")) {
                    stb_c_lexer_get_token(&lex); // derive_json
                    stb_c_lexer_get_token(&lex); // (
                    stb_c_lexer_get_token(&lex); // )
                    strct.derived_schemas |= DERIVE_JSON;
                
            }
            push_da(out, strct);
        } else {
            lex.parse_point = pp;
            stb_c_lexer_get_token(&lex);
        }
    }
}

#define fmt(...) (sprintf(tmp_str, __VA_ARGS__), tmp_str)

char tmp_str[4096];
int main(int argc, char** argv) {
    bool derive_all = false;
    char* root_dir;
    DIR* pwd;
    // TODO: Make a proper flag system instead of this mess
    if (argc > 1) {
        // normally you should escape this but who cares tbh
        root_dir = argv[1];
        pwd = opendir(root_dir);
        if (!pwd) {
            root_dir = "./examples";
            pwd = opendir(root_dir);
            if (!pwd) return 1;
            if (strcmp(argv[1], "--derive-all") == 0) {
                derive_all = true;
            }
        };
        if (argc > 2) {
            if (strcmp(argv[2], "--derive-all") == 0) {
                derive_all = true;
            }
        }
    } else {
        root_dir = "./examples";
        pwd = opendir(root_dir);
        if (!pwd) return 1;
    }

    CCompounds types = {0}; // leaks (static data)
    String file_buf = {0}; // leaks (static data)
    for (struct dirent* ent = readdir(pwd); ent; ent = readdir(pwd)) {
        if (ent->d_type == DT_REG) {
            if (
                strcmp(ent->d_name + strlen(ent->d_name) - 2, ".c") == 0 ||
                strcmp(ent->d_name + strlen(ent->d_name) - 2, ".h") == 0
            ) {
                printf("Analyzing file: %s\n", ent->d_name);
                file_buf.len = 0;
                read_entire_file(fmt("%s/%s", root_dir, ent->d_name), &file_buf); // TODO: check errors
                analyze_file(file_buf, &types, derive_all);
            }
        }
    }
    printf("Analayzed %lu types.\n", types.len);
    String book_buf = {0};
    FILE* book_header = fopen("bookkeeper.h", "w");
    fprintf(book_header, "#define derive_json(...)\n");
    fclose(book_header);
    FILE* book_impl = fopen("bookkeeper.c", "w"); // TODO: check errors
    print_string(&book_buf, "#ifndef BK_FMT\n");
    print_string(&book_buf, "#define BK_FMT(...) offset += sprintf(dst + offset, __VA_ARGS__)\n");
    print_string(&book_buf, "#endif\n");
    print_string(&book_buf, "#ifndef BK_OFFSET_t\n");
    print_string(&book_buf, "#define BK_OFFSET_t size_t\n");
    print_string(&book_buf, "#endif\n");
    for (size_t i = 0; i < types.len; ++i) {
        CCompound * ty = types.items + i;
        if (ty->derived_schemas & DERIVE_JSON) {
            print_string(&book_buf, "\nvoid dump_json_%s(%s* item, void* dst) {\n", ty->name, ty->name);
            print_string(&book_buf, "    BK_OFFSET_t offset = 0;\n");
            print_string(&book_buf, "    BK_FMT(\"{\");\n");
            for (size_t j = 0; j < ty->fields.len; ++j) {
                Field* f = ty->fields.items + j;
                switch (f->type.kind) {
                case CPRIMITIVE: {
                    switch (f->type.type) {
                    case CINT: {
                        print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":%%d\", item->%s);\n", f->name, f->name);
                    } break;
                    case CUINT: {
                        print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":%%u\", item->%s);\n", f->name, f->name);
                    } break;
                    case CLONG: {
                        print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":%%ld\", item->%s);\n", f->name, f->name);
                    } break;
                    case CULONG: {
                        print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":%%lu\", item->%s);\n", f->name, f->name);
                    } break;
                    case CFLOAT: {
                        print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":%%f\", item->%s);\n", f->name, f->name);
                    } break;
                    case CBOOL: {
                        print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":%%s\", item->%s ? \"true\" : \"false\");\n", f->name, f->name);
                    } break;
                    case CSTRING: {
                        // TODO: The generated code should escape item->field before printing it
                        print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":\\\"%%s\\\"\", item->%s);\n", f->name, f->name);
                    } break;
                    case CCHAR: {
                        print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":%%c\", item->%s);\n", f->name, f->name);
                    } break;
                    default: abort();
                    }
                } break;
                case CEXTERNAL: {
                    print_string(&book_buf, "    BK_FMT(\"\\\"%s\\\":\");\n", f->name);
                    print_string(&book_buf, "    dump_json_%s(&item->%s, dst);\n", f->type.name, f->name);
                } break;
                default: abort();
                }
                if (j < ty->fields.len - 1) print_string(&book_buf, "    BK_FMT(\",\");\n");
            }
            print_string(&book_buf, "    BK_FMT(\"}\");\n");
            print_string(&book_buf, "}");
        }
    }
    push_da(&book_buf, '\n');
    fwrite(book_buf.items, sizeof *book_buf.items, book_buf.len, book_impl); // TODO: check errors
    fclose(book_impl);
    return 0;
}


