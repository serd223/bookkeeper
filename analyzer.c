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
    CCOMPOUND = 1,
    CPRIMITIVE,
    CEXTERNAL
} CType_Kind;

typedef enum {
    CINT = 1,
    CFLOAT,
    CBOOL,
    CSTRING
} CPrimitive;

typedef struct __CType CType;
typedef struct __Field Field;

typedef struct {
    Field* items;
    size_t len;
    size_t cap;
} Fields;

typedef struct {
    CType* items;
    size_t len;
    size_t cap;
} CTypes;

struct __CType {
    CType_Kind kind;
    Fields fields;    // CCOMPOUND
    const char* name; // CCOMPOUND || CEXTERNAL
    CPrimitive type;  // CPIRIMITIVE
};

struct __Field{
    const char* name;
    CType type;
};

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
    (arr)->items[(arr)->len++] = (item);                                            \

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

bool peek_expect_ids(stb_lexer* lex, int ids_count, ...) {
    va_list args;
    va_start(args, ids_count);
    char* pp = lex->parse_point;
    bool res = va_get_expect_ids(lex, ids_count, args);
    lex->parse_point = pp;
    va_end(args);
    return res;
}

bool peek_expect_tokens(stb_lexer* lex, int ids_count, ...) {
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

void analyze_file(String content, CTypes* types) {
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
            CType strct = {0};
            strct.kind = CCOMPOUND;
            for (;;) {
                Field field = {0};
                if (peek_expect_tokens(&lex, 5, CLEX_id, CLEX_id, '*', CLEX_id, ';') && peek_expect_ids(&lex, 2, "const", "char")) { 
                    stb_c_lexer_get_token(&lex); // const
                    stb_c_lexer_get_token(&lex); // char
                    stb_c_lexer_get_token(&lex); // *
                    stb_c_lexer_get_token(&lex); // name
                    field.name = strdup(lex.string); // leaks
                    stb_c_lexer_get_token(&lex); // ;
                    field.type.kind = CPRIMITIVE;
                    field.type.type = CSTRING;
                    push_da(&strct.fields, field);

                } else if (peek_expect_tokens(&lex, 4, CLEX_id, '*', CLEX_id, ';') && peek_expect_ids(&lex, 1, "char")) {
                    stb_c_lexer_get_token(&lex); // char
                    stb_c_lexer_get_token(&lex); // *
                    stb_c_lexer_get_token(&lex); // name
                    field.name = strdup(lex.string); // leaks
                    stb_c_lexer_get_token(&lex); // ;
                    field.type.kind = CPRIMITIVE;
                    field.type.type = CSTRING;
                    push_da(&strct.fields, field);
                } else if (peek_expect_tokens(&lex, 3, CLEX_id, CLEX_id, ';')) {
                    bool is_known_primitive = true;
                    if (peek_expect_ids(&lex, 1, "int")) {
                        field.type.type = CINT;
                    } else if (peek_expect_ids(&lex, 1, "double")) {
                        field.type.type = CFLOAT;
                    } else if (peek_expect_ids(&lex, 1, "bool")) {
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
                    break;
                }
            }
            // `continue`ing here can leak unused field/type names of partially correct structs
            if (!get_expect_tokens(&lex, 2, '}', CLEX_id)) continue;
            strct.name = strdup(lex.string); // leaks (static data)
            push_da(types, strct);
        } else {
            lex.parse_point = pp;
            stb_c_lexer_get_token(&lex);
        }
    }
}

#define fmt(...) (sprintf(tmp_str, __VA_ARGS__), tmp_str)

char tmp_str[1024];
int main(int argc, char** argv) {
    DIR* pwd;
    if (argc > 1) {
        // normally you should escape this but who cares tbh
        pwd = opendir(argv[1]);
    } else {
        pwd = opendir("./examples");
    }
    if (!pwd) return 1;

    CTypes types = {0}; // leaks (static data)
    String file_buf = {0}; // leaks (static data)
    for (struct dirent* ent = readdir(pwd); ent; ent = readdir(pwd)) {
        if (ent->d_type == DT_REG) {
            if (
                strcmp(ent->d_name + strlen(ent->d_name) - 2, ".c") == 0 ||
                strcmp(ent->d_name + strlen(ent->d_name) - 2, ".h") == 0
            ) {
                printf("Analyzing file: %s\n", ent->d_name);
                file_buf.len = 0;
                read_entire_file(fmt("./examples/%s", ent->d_name), &file_buf);
                analyze_file(file_buf, &types);
            }
        }
    }
    printf("Analayzed %lu types.\n", types.len);
    return 0;
}


