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
    char* data;
    size_t count;
    size_t cap;
} String;

void push_string(String* s, char c) {
    if (s->count + 1 >= s->cap) {
        if (s->cap > 0) {
            s->cap *= 2;
        } else {
            s->cap = 2;
        }
        if (s->data) {
            s->data = realloc(s->data, s->cap);
        } else {
            s->data = malloc(s->cap);
        }
    }
    s->data[s->count++] = c;
}

void read_entire_file(const char* file_name, String* out) {
    struct stat s = {0};
    stat(file_name, &s);
    size_t f_len = s.st_size / sizeof(char);
    if (f_len >= (out->cap - out->count)) {
        out->cap += f_len; // allocate extra space beacuse why not ram is cheap
        if (out->data) {
            out->data = realloc(out->data, out->cap);
        } else {
            out->data = malloc(out->cap);
        }
    }
    FILE* f = fopen(file_name, "rb");
    out->count += fread(out->data + out->count, sizeof(char), f_len, f) / sizeof(char);
}

bool expect_ids(stb_lexer* lex, int ids_count, ...) {
    va_list args;
    va_start(args, ids_count);
    for (size_t i = 0; i < ids_count; ++i) {
        if (!(lex->token == CLEX_id))                            goto _false;
        if (strcmp(lex->string, va_arg(args, const char*)) != 0) goto _false;
        stb_c_lexer_get_token(lex);
    }

    goto _true;
    _false:
    va_end(args);
    return false;
    _true:
    va_end(args);
    return true;
}

void analyze_file(String content) {
    static char string_store[4096];
    stb_lexer lex = {0};
    stb_c_lexer_init(&lex, content.data, content.data + content.count, string_store, sizeof string_store);
    for (int tok;;) {
        stb_c_lexer_get_token(&lex);
        tok = lex.token;
        if (!tok || tok == CLEX_eof) break;
        char* pp = lex.parse_point;
        if (expect_ids(&lex, 2, "typedef", "struct")) {
            stb_c_lexer_get_token(&lex);
            if (!(lex.token == '{')) break; // NOTE: this tool can only understand typedef struct {} Name; style definitions
            // TODO: parse field
        } else {
            lex.parse_point = pp;
        }
    }
}

int main(void) {
    DIR* pwd = opendir(".");
    if (!pwd) return 1;
    String file_buf = {0}; // leaks
    for (struct dirent* ent = readdir(pwd); ent; ent = readdir(pwd)) {
        if (ent->d_type == DT_REG) {
            if (
                strcmp(ent->d_name + strlen(ent->d_name) - 2, ".c") == 0 ||
                strcmp(ent->d_name + strlen(ent->d_name) - 2, ".h") == 0
            ) {
                printf("analyzing file: %s\n", ent->d_name);
                file_buf.count = 0;
                read_entire_file(ent->d_name, &file_buf);
                analyze_file(file_buf);
            }
        }
    }
    return 0;
}


