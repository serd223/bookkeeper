/*
Copyright (c) 2025 Serdar Çoruhlu <serdar.coruhlu@hotmail.com>

Permission is hereby granted, free of charge, to any
person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the
Software without restriction, including without
limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice
shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"
#ifdef DEBUG
#define printf(...)
#endif

// NOTE: in order to make this tool still have one-file source code, we have to _manually_
// sync these definitions with the ones inside `bookkeeper_gen_ext.h` for consistency.
// You should only put definitions inside the scope of this guard if that definition needs to
// be accessible inside extensions.
#ifndef __BK_GEN_EXT_DEFINITIONS
typedef struct {
    bool silent;
    char* gen_fmt_macro;
    char* gen_implementation_macro;
    char* gen_fmt_dst_macro;
    char* disable_dump_macro;
    char* disable_parse_macro;
    char* offset_type_macro;
    char* type_disable_macro_prefix;
    bool derive_all;
    char* input_path;
    char* out_path;
} BkState;

static BkState bk = {0};
static char tmp_str[4096];
#define tfmt(...) (sprintf(tmp_str, __VA_ARGS__), tmp_str)
#define fmt(...) strdup(tfmt(__VA_ARGS__))

#define bk_log_loc(level, source, line, ...) do {\
    if (!bk.silent) {\
        switch (level) {\
        case LOG_INFO: {\
            fprintf(stderr, "%s:%d: [INFO] ", source, line);\
        } break;\
        case LOG_WARN: {\
            fprintf(stderr, "%s:%d: [WARN] ", source, line);\
        } break;\
        case LOG_ERROR: {\
            fprintf(stderr, "%s:%d: [ERROR] ", source, line);\
        } break;\
        default: abort();\
        }\
        fprintf(stderr, __VA_ARGS__);\
    }\
} while(0)

#define bk_log(level, ...) bk_log_loc(level, __FILE__, __LINE__, __VA_ARGS__)

typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} Log_Level;

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

typedef struct {
    int derived_schemas; // TODO: this bitfield method puts a (somewhat low) hard limit on the amount of allowed schmeas
    Fields fields;
    const char* name;
} CCompound;

typedef struct {
    CCompound* items;
    size_t len;
    size_t cap;
} CCompounds;

#define push_da(arr, item) do {                                                     \
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
} while(0)

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
#endif // __BK_GEN_EXT_DEFINITIONS

typedef struct {
    size_t (*gen_dump_decl)(String* book_buf, CCompound* ty, const char* dst_type);
    size_t (*gen_parse_decl)(String* book_buf, CCompound* ty);
    void (*gen_dump_impl)(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);
    void (*gen_parse_impl)(String* book_buf, CCompound* ty);
    const char* derive_attr;
} Schema;

typedef struct {
    Schema* items;
    size_t len;
    size_t cap;
} Schemas;

// General utility functions
bool read_entire_file_loc(const char* file_name, String* dst, const char* source_file, int source_line);
#define read_entire_file(file_name, dst) read_entire_file_loc(file_name, dst, __FILE__, __LINE__)
bool write_entire_file_loc(const char* file_name, String* src, const char* source_file, int source_line);
#define write_entire_file(file_name, src) write_entire_file_loc(file_name, src, __FILE__, __LINE__)
unsigned long djb2(const char* s);

// Parsing C code
bool va_get_expect_ids(stb_lexer* lex, va_list args);
bool va_get_expect_tokens(stb_lexer* lex, va_list args);
bool peek_ids(stb_lexer* lex, ...);
bool peek_tokens(stb_lexer* lex, ...);
bool get_expect_tokens(stb_lexer* lex, ...);
bool get_expect_ids(stb_lexer* lex,...);
void analyze_file(Schemas schemas, String content, CCompounds* out, bool derive_all);

// Code generation
size_t gen_dump_decl(Schemas schemas, String* book_buf, CCompound* ty, const char* dst_type, const char* disable_dump_macro);
size_t gen_parse_decl(Schemas schemas, String* book_buf, CCompound* ty, const char* disable_parse_macro);
void gen_dump_impl(Schemas schemas, String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro, const char* disable_dump_macro);
void gen_parse_impl(Schemas schemas, String* book_buf, CCompound* ty, const char* disable_parse_macro);

// JSON generation
size_t gen_json_dump_decl(String* book_buf, CCompound* ty, const char* dst_type);
size_t gen_json_parse_decl(String* book_buf, CCompound* ty);
void gen_json_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);
void gen_json_parse_impl(String* book_buf, CCompound* ty);

// Debug generation
size_t gen_debug_dump_decl(String* book_buf, CCompound* ty, const char* dst_type);
void gen_debug_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);

// Command line arguments
typedef struct {
    char* name;
    char* flag;
    char* usage;
    char* desc;
    bool (*exec_c)(BkState* bk, int* i, int argc, char** argv);
} Command;

// If only we had closures in C...
bool help_cmd(BkState* bk, int* i, int argc, char** argv);
bool search_directory_cmd(BkState* bk, int* i, int argc, char** argv);
bool output_directory_cmd(BkState* bk, int* i, int argc, char** argv);
bool silent_cmd(BkState* bk, int* i, int argc, char** argv);
bool derive_all_cmd(BkState* bk, int* i, int argc, char** argv);
bool gen_impl_cmd(BkState* bk, int* i, int argc, char** argv);
bool gen_fmt_dst_cmd(BkState* bk, int* i, int argc, char** argv);
bool gen_fmt_cmd(BkState* bk, int* i, int argc, char** argv);
bool disable_dump_cmd(BkState* bk, int* i, int argc, char** argv);
bool disable_parse_cmd(BkState* bk, int* i, int argc, char** argv);
bool offset_type_cmd(BkState* bk, int* i, int argc, char** argv);

#define exec_cmd(cmd)\
((cmd)->exec_c ? ((cmd)->exec_c(&bk, &i, argc, argv) ? true : (printf("Usage of '%s': %s\n", (cmd)->name, (cmd)->usage), false)) : false)

static Command commands[] = {
    {
        .name = "help",
        .flag = "-h",
        .usage = "-h <command (optional)>",
        .desc = "Prints a list of all commands or information about the provided command",
        .exec_c = help_cmd
    },
    // {
    //     .name = "search-file",
    //     .flag = "-f",
    //     .usage = "-f <file>",
    //     .desc = "TODO",
    //     .exec_c = NULL // TODO: search-file
    // },
    {
        .name = "search-directory",
        .flag = "-d",
        .usage = "-d <dir>",
        .desc = "The provided directory will be searched for '.c' or '.h' files to analyze",
        .exec_c = search_directory_cmd
    },
    // {
    //     .name = "output-file",
    //     .flag = "-of",
    //     .usage = "-of <file>",
    //     .desc = "TODO",
    //     .exec_c = NULL // TODO: output-file
    // },
    {
        .name = "output-directory",
        .flag = "-od",
        .usage = "-od <dir>",
        .desc = "All generated files will be placed inside the provided directory",
        .exec_c = output_directory_cmd
    },
    {
        .name = "silent",
        .flag = "--silent",
        .usage = "--silent",
        .desc = "Turns off logging",
        .exec_c = silent_cmd
    },
    {
        .name = "derive-all",
        .flag = "--derive-all",
        .usage = "--derive-all",
        .desc = "Derives all possible schemas for all analyzed structs",
        .exec_c = derive_all_cmd
    },
    {
        .name = "gen-implementation",
        .flag = "--gen-implementation",
        .usage = "--gen-implementation <name>",
        .desc = "Sets the macro that will be used in the generated code to control enabling implementation",
        .exec_c = gen_impl_cmd
    },
    {
        .name = "gen-fmt-dst",
        .flag = "--gen-fmt-dst",
        .usage = "--gen-fmt-dst <name>",
        .desc = "Sets the macro that will be used in the generated code to control the type of `dst` in `dump` functions",
        .exec_c = gen_fmt_dst_cmd
    },
    {
        .name = "gen-fmt",
        .flag = "--gen-fmt",
        .usage = "--gen-fmt <name>",
        .desc = "Sets the macro that will be used in the generated `dump` functions to output with `printf` style arguments",
        .exec_c = gen_fmt_cmd
    },
    {
        .name = "disable-dump",
        .flag = "--disable-dump",
        .usage = "--disable-dump <name>",
        .desc = "Sets the macro that will be used in the generated code to disable `dump` functions",
        .exec_c = disable_dump_cmd
    },
    {
        .name = "disable-parse",
        .flag = "--disable-parse",
        .usage = "--disable-parse <name>",
        .desc = "Sets the macro that will be used in the generated code to disable `parse` functions",
        .exec_c = disable_parse_cmd
    },
    {
        .name = "offset-type",
        .flag = "--offset-type",
        .usage = "--offset-type <name>",
        .desc = "Sets the macro that will be used in the generated code to control the type of the `offset` variable inside `dump` functions",
        .exec_c = offset_type_cmd
    },
    {
        .name = "disable-type-prefix",
        .flag = "--disable-type-prefix",
        .usage = "--disable-type-prefix <name>",
        .desc = "Sets the prefix of the generated macro that can be used to disable generated code for a specific user type (<prefix><typename>)",
        .exec_c = offset_type_cmd
    },
};
const size_t commands_count = sizeof commands / sizeof *commands;

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("Basic usage: %s -d <search-directory> -od <output-directory>\n", argv[0]);
        printf("Use `-h` to print all available commands, `-h <command-name>` to see that command's usage.\n");
        return 0;
    }
    
    Schemas schemas = {0};
    Schema json = {
        .gen_dump_decl = gen_json_dump_decl, 
        .gen_parse_decl = gen_json_parse_decl, 
        .gen_dump_impl = gen_json_dump_impl, 
        .gen_parse_impl = gen_json_parse_impl, 
        .derive_attr = "derive_json"
    };
    Schema debug = {
        .gen_dump_decl = gen_debug_dump_decl, 
        .gen_parse_decl = NULL, 
        .gen_dump_impl = gen_debug_dump_impl, 
        .gen_parse_impl = NULL, 
        .derive_attr = "derive_debug"
    };
    push_da(&schemas, json);
    push_da(&schemas, debug);

    // Do whatever you want here
    #ifdef BK_ADD_SCHEMAS
    do {
        BK_ADD_SCHEMAS(schemas)
    } while (0);
    #endif
    bk.gen_fmt_macro = "BK_FMT";
    bk.gen_implementation_macro = "BK_IMPLEMENTATION";
    bk.gen_fmt_dst_macro = "BK_FMT_DST_t";
    bk.disable_dump_macro = "DISABLE_DUMP";
    bk.disable_parse_macro = "DISABLE_PARSE";
    bk.offset_type_macro = "BK_OFFSET_t";
    bk.type_disable_macro_prefix = "BK_DISABLE_";

    bk.derive_all = false;
    bk.input_path = NULL;
    bk.out_path = NULL;

    // NOTE: Assumes that `argv` lives long enough
    for (int i = 1; i < argc; ++i) {
        for (size_t j = 0; j < commands_count; ++j) {
            if (strcmp(commands[j].flag, argv[i]) == 0) {
                if (!exec_cmd(&commands[j])) {
                    return 1;
                }
            }
        }
    }

    if (!bk.input_path) {
        bk_log(LOG_WARN, "No search path set, exiting...\n");
        return 1;
    }
    if (!bk.out_path) {
        bk_log(LOG_WARN, "No output path set, exiting...\n");
        return 1;
    }
    
    {
        size_t in_len = strlen(bk.input_path);
        if (bk.input_path[in_len - 1] == '/') bk.input_path[in_len - 1] = 0;
        size_t out_len = strlen(bk.out_path);
        if (bk.out_path[out_len - 1] == '/') bk.out_path[out_len - 1] = 0;
    }


    bk_log(LOG_INFO, "Number of registered schemas: %lu\n", schemas.len);

    String book_buf = {0};
    print_string(&book_buf, "#ifndef __DERIVES_H__\n");
    print_string(&book_buf, "#define __DERIVES_H__\n");
    print_string(&book_buf, "#define derive_all(...)\n");
    for (size_t i = 0; i < schemas.len; ++i) {
        print_string(&book_buf, "#define %s(...)\n", schemas.items[i].derive_attr);
    }
    print_string(&book_buf, "#endif // __DERIVES_H__\n");
    write_entire_file(tfmt("%s/derives.h", bk.out_path), &book_buf);

    CCompounds types = {0}; // leaks (static data)
    String file_buf = {0}; // leaks (static data)
    book_buf.len = 0;
    DIR* input_dir = opendir(bk.input_path); // TODO: this is our main POSIX-dependent piece of code, perhaps write a cross-platform wrapper 
    size_t file_idx = 0;
    if (!input_dir) {
        bk_log(LOG_ERROR, "Couldn't open directory '%s': %s\n", bk.input_path, strerror(errno));
        return 1;
    }
    for (struct dirent* ent = readdir(input_dir); ent; ent = readdir(input_dir)) {
        if (ent->d_type == DT_REG) {
            if (
                strcmp(ent->d_name + strlen(ent->d_name) - 2, ".c") == 0 ||
                strcmp(ent->d_name + strlen(ent->d_name) - 2, ".h") == 0
            ) {
                char* in_file = fmt("%s/%s", bk.input_path, ent->d_name);
                unsigned long in_hash = djb2(in_file);

                bk_log(LOG_INFO, "Analyzing file: %s\n", ent->d_name);
                file_buf.len = 0;
                if (!read_entire_file(in_file, &file_buf)) continue;

                types.len = 0;
                analyze_file(schemas, file_buf, &types, bk.derive_all);
                bk_log(LOG_INFO, "Analayzed %lu type(s).\n", types.len);
                book_buf.len = 0;
                if (types.len > 0) {
                    print_string(&book_buf, "#ifndef __BK_%lu_%lu_H__ // Generated from: %s\n", in_hash, file_idx, in_file);
                    print_string(&book_buf, "#define __BK_%lu_%lu_H__\n", in_hash, file_idx);
                    print_string(&book_buf, "#ifndef %s\n", bk.gen_fmt_dst_macro);
                    print_string(&book_buf, "#define %s FILE*\n", bk.gen_fmt_dst_macro);
                    print_string(&book_buf, "#endif // %s\n", bk.gen_fmt_dst_macro);
                    print_string(&book_buf, "#ifndef %s\n", bk.gen_fmt_macro);
                    print_string(&book_buf, "#define %s(...) offset += fprintf(dst, __VA_ARGS__)\n", bk.gen_fmt_macro);
                    print_string(&book_buf, "#endif // %s\n", bk.gen_fmt_macro);
                    print_string(&book_buf, "#ifndef %s\n", bk.offset_type_macro);
                    print_string(&book_buf, "#define %s size_t\n", bk.offset_type_macro);
                    print_string(&book_buf, "#endif // %s\n", bk.offset_type_macro);
                    size_t num_decls = 0;
                    for (size_t i = 0; i < types.len; ++i) {
                        print_string(&book_buf, "\n#ifndef %s%s\n", bk.type_disable_macro_prefix, types.items[i].name);
                        num_decls += gen_dump_decl(schemas, &book_buf, types.items + i, bk.gen_fmt_dst_macro, bk.disable_dump_macro);
                        num_decls += gen_parse_decl(schemas, &book_buf, types.items + i, bk.disable_parse_macro);
                        print_string(&book_buf, "\n#endif // %s%s\n", bk.type_disable_macro_prefix, types.items[i].name);
                    }
                    if (num_decls > 0) {
                        print_string(&book_buf, "\n#ifdef %s\n", bk.gen_implementation_macro);
                        for (size_t i = 0; i < types.len; ++i) {
                            print_string(&book_buf, "\n#ifndef %s%s\n", bk.type_disable_macro_prefix, types.items[i].name);
                            gen_dump_impl(schemas, &book_buf, types.items + i, bk.gen_fmt_dst_macro, bk.gen_fmt_macro, bk.disable_dump_macro);
                            gen_parse_impl(schemas, &book_buf, types.items + i, bk.disable_parse_macro);
                            print_string(&book_buf, "\n#endif // %s%s\n", bk.type_disable_macro_prefix, types.items[i].name);
                        }
                        print_string(&book_buf, "\n#endif // %s\n", bk.gen_implementation_macro);

                        push_da(&book_buf, '\n');
                        print_string(&book_buf, "#endif // __BK_%lu_%lu_H__\n", in_hash, file_idx);
                        char* out_file = tfmt("%s/%s.bk.h", bk.out_path, ent->d_name);
                        write_entire_file(out_file, &book_buf);
                        file_idx += 1; // increment index counter even if write_entire_file errors just to be safe
                    }
                }
                free(in_file);
            }
        }
    }
    closedir(input_dir);
    return 0;
}

bool read_entire_file_loc(const char* file_name, String* dst, const char* source_file, int source_line) {
    struct stat s = {0};
    stat(file_name, &s);
    size_t f_len = s.st_size / sizeof(char);
    FILE* f = fopen(file_name, "rb");
    if (f == NULL) {
        bk_log_loc(LOG_ERROR, source_file, source_line, "Couldn't open file '%s': %s\n", file_name, strerror(errno));
        return false;
    }
    if (f_len >= (dst->cap - dst->len)) {
        dst->cap += f_len; // allocate extra space beacuse why not ram is cheap
        if (dst->items) {
            dst->items = realloc(dst->items, dst->cap);
        } else {
            dst->items = malloc(dst->cap);
        }
    }
    size_t bytes_read = fread(dst->items + dst->len, sizeof(char), f_len, f) / sizeof(char);
    if (bytes_read <= 0) {
        if (ferror(f)) {
            bk_log_loc(LOG_ERROR, source_file, source_line, "Couldn't read from file '%s': %s\n", file_name, strerror(errno));
            fclose(f);
            return false;
        }
    }
    dst->len += bytes_read; 
    fclose(f);
    return true;
}

bool write_entire_file_loc(const char* file_name, String* src, const char* source_file, int source_line) {
    FILE* f = fopen(file_name, "w");
    if (f == NULL) {
        bk_log_loc(LOG_ERROR, source_file, source_line, "Couldn't open file '%s': %s\n", file_name, strerror(errno));
        return false;
    }
    if (fwrite(src->items, sizeof *src->items, src->len, f) <= 0) {
        if (ferror(f)) {
            bk_log_loc(LOG_ERROR, source_file, source_line, "Couldn't write to file '%s': %s\n", file_name, strerror(errno));
            fclose(f);
            return false;
        }
    } 
    fclose(f);
    bk_log_loc(LOG_INFO, source_file, source_line, "Generated file: %s\n", file_name);
    return true;
}

unsigned long djb2(const char* s) {
    unsigned long hash = 5381;
    for (char c; (c = *s++);) hash = ((hash << 5) + hash) + (unsigned long) c;
    return hash;
}


char* extract_positional_impl(int* i, int argc, char** argv, char* flag_name, ...) {
    *i = *i + 1;
    if (*i < argc) {
        return argv[*i];
    }
    printf("Flag '%s' requires argument(s):", flag_name);
    va_list args;
    va_start(args, flag_name);
    
    for(char* arg = va_arg(args, char*); arg != NULL; arg = va_arg(args, char*)) {
        printf(" <%s>", arg);
    }
    printf("\n");

    va_end(args);
    return NULL;
}

bool va_get_expect_ids(stb_lexer* lex, va_list args) {
    const char* expected = va_arg(args, const char*);
    for (; expected != NULL; expected = va_arg(args, const char*)) {
        stb_c_lexer_get_token(lex);
        if (!(lex->token == CLEX_id))           return false;
        if (strcmp(lex->string, expected) != 0) return false;
    }
    return true;
}

bool va_get_expect_tokens(stb_lexer* lex, va_list args) {
    int expected = va_arg(args, int);
    for (; expected > -1 ; expected = va_arg(args, int)) {
        stb_c_lexer_get_token(lex);
        if (!(lex->token == expected)) return false;
    }
    return true;
}

/// Arguments should be of type `const char*` and end with NULL
bool peek_ids(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    char* pp = lex->parse_point;
    bool res = va_get_expect_ids(lex, args);
    lex->parse_point = pp;
    va_end(args);
    return res;
}

/// Arguments should be of type `int` and end with -1
bool peek_tokens(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    char* pp = lex->parse_point;
    bool res = va_get_expect_tokens(lex, args);
    lex->parse_point = pp;
    va_end(args);
    return res;
}

/// Arguments should be of type `int` and end with -1
bool get_expect_tokens(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    bool res = va_get_expect_tokens(lex, args);
    va_end(args);
    return res;
}

/// Arguments should be of type `const char*` and end with NULL
bool get_expect_ids(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    bool res = va_get_expect_ids(lex, args);
    va_end(args);
    return res;
}

void analyze_file(Schemas schemas, String content, CCompounds* out, bool derive_all) {
    static char string_store[4096];
    stb_lexer lex = {0};
    stb_c_lexer_init(&lex, content.items, content.items + content.len, string_store, sizeof string_store);
    for (;;) {
        char* pp = lex.parse_point;
        stb_c_lexer_get_token(&lex);
        if (!lex.token || lex.token == CLEX_eof) break;
        lex.parse_point = pp;
        // NOTE: this tool can only understand typedef struct {} Name; style definitions
        if (get_expect_ids(&lex, "typedef", "struct", NULL)) {
            if (!(get_expect_tokens(&lex, '{', -1))) continue;
            CCompound strct = {0};
            if (derive_all) strct.derived_schemas |= UINT_MAX;
            for (;;) {
                Field field = {0};
                if (peek_tokens(&lex, CLEX_id, CLEX_id, '*', CLEX_id, ';', -1) && peek_ids(&lex, "const", "char", NULL)) { 
                    stb_c_lexer_get_token(&lex); // const
                    stb_c_lexer_get_token(&lex); // char
                    stb_c_lexer_get_token(&lex); // *
                    stb_c_lexer_get_token(&lex); // name
                    field.name = strdup(lex.string); // leaks
                    stb_c_lexer_get_token(&lex); // ;
                    field.type.kind = CPRIMITIVE;
                    field.type.type = CSTRING;
                    push_da(&strct.fields, field);

                } else if (peek_tokens(&lex, CLEX_id, '*', CLEX_id, ';', -1) && peek_ids(&lex, "char", NULL)) {
                    stb_c_lexer_get_token(&lex); // char
                    stb_c_lexer_get_token(&lex); // *
                    stb_c_lexer_get_token(&lex); // name
                    field.name = strdup(lex.string); // leaks
                    stb_c_lexer_get_token(&lex); // ;
                    field.type.kind = CPRIMITIVE;
                    field.type.type = CSTRING;
                    push_da(&strct.fields, field);
                } else if (peek_tokens(&lex, CLEX_id, CLEX_id, CLEX_id, ';', -1)) {
                    bool is_known_primitive = true;
                    if (peek_ids(&lex, "unsigned", "int", NULL)) {
                        field.type.type = CUINT;
                    } else if (peek_ids(&lex, "unsigned", "long", NULL)) {
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
                    
                } else if (peek_tokens(&lex, CLEX_id, CLEX_id, ';', -1)) {
                    bool is_known_primitive = true;
                    if (peek_ids(&lex, "int", NULL)) {
                        field.type.type = CINT;
                    } else if (peek_ids(&lex, "long", NULL)) {
                        field.type.type = CLONG;
                    } else if (peek_ids(&lex, "size_t", NULL)) {
                        field.type.type = CULONG;
                    } else if (peek_ids(&lex, "double", NULL) || peek_ids(&lex, "float", NULL)) {
                        field.type.type = CFLOAT;
                    } else if (peek_ids(&lex, "char", NULL)) {
                        field.type.type = CCHAR;
                    } else if (peek_ids(&lex, "double", NULL) || peek_ids(&lex, "float", NULL)) {
                        field.type.type = CFLOAT;
                    } else if (peek_ids(&lex, "bool", NULL)) {
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
            if (!get_expect_tokens(&lex, '}', CLEX_id, -1)) continue;
            strct.name = strdup(lex.string); // leaks (static data)
            while (peek_tokens(&lex, CLEX_id, '(', ')', -1)) {
                bool matched = false;

                // Special attribute(s)
                if (peek_ids(&lex, "derive_all", NULL)) {
                    strct.derived_schemas |= UINT_MAX;
                    matched = true;
                } else {
                    for (size_t i = 0; i < schemas.len; ++i) {
                        Schema* schema = schemas.items + i;
                        if (peek_ids(&lex, schema->derive_attr, NULL)) {
                            strct.derived_schemas |= (1 << i);
                            matched = true;
                            break;
                        }
                    }
                }

                if (matched) {
                    stb_c_lexer_get_token(&lex); // derive_schema
                    stb_c_lexer_get_token(&lex); // (
                    stb_c_lexer_get_token(&lex); // )
                }
                
            }
            push_da(out, strct);
        } else {
            lex.parse_point = pp;
            stb_c_lexer_get_token(&lex);
        }
    }
}

size_t gen_dump_decl(Schemas schemas, String* book_buf, CCompound* ty, const char* dst_type, const char* disable_dump_macro) {
    size_t count = 0;
    if (ty->derived_schemas == 0) return count;
    print_string(book_buf, "\n#ifndef %s\n", disable_dump_macro);
    for (size_t i = 0; i < schemas.len; ++i) {
        Schema* schema = schemas.items + i;
        if (ty->derived_schemas & (1 << i)) {
            if (schema->gen_dump_decl != NULL) count += schema->gen_dump_decl(book_buf, ty, dst_type);
        }
    }
    print_string(book_buf, "#endif // %s\n", disable_dump_macro);
    return count;
}

size_t gen_parse_decl(Schemas schemas, String* book_buf, CCompound* ty, const char* disable_parse_macro) {
    size_t count = 0;
    if (ty->derived_schemas == 0) return count;
    print_string(book_buf, "\n#ifndef %s\n", disable_parse_macro);
    for (size_t i = 0; i < schemas.len; ++i) {
        Schema* schema = schemas.items + i;
        if (ty->derived_schemas & (1 << i)) {
            if (schema->gen_parse_decl != NULL) count += schema->gen_parse_decl(book_buf, ty);
        }
    }
    print_string(book_buf, "#endif // %s\n", disable_parse_macro);
    return count;
}

void gen_dump_impl(Schemas schemas, String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro, const char* disable_dump_macro) {
    if (ty->derived_schemas == 0) return;
    print_string(book_buf, "\n#ifndef %s\n", disable_dump_macro);
    for (size_t i = 0; i < schemas.len; ++i) {
        Schema* schema = schemas.items + i;
        if (ty->derived_schemas & (1 << i)) {
            if (schema->gen_dump_impl != NULL) schema->gen_dump_impl(book_buf, ty, dst_type, fmt_macro);
        }
    }
    print_string(book_buf, "\n#endif // %s\n", disable_dump_macro);
}

void gen_parse_impl(Schemas schemas, String* book_buf, CCompound* ty, const char* disable_parse_macro) {
    if (ty->derived_schemas == 0) return;
    print_string(book_buf, "\n#ifndef %s\n", disable_parse_macro);
    for (size_t i = 0; i < schemas.len; ++i) {
        Schema* schema = schemas.items + i;
        if (ty->derived_schemas & (1 << i)) {
            if (schema->gen_parse_impl != NULL) schema->gen_parse_impl(book_buf, ty);
        }
    }
    print_string(book_buf, "\n#endif // %s\n", disable_parse_macro);
}

// JSON generation
size_t gen_json_dump_decl(String* book_buf, CCompound* ty, const char* dst_type) {
    print_string(book_buf, "void dump_json_%s(%s* item, %s dst);\n", ty->name, ty->name, dst_type);
    return 1;
}
size_t gen_json_parse_decl(String* book_buf, CCompound* ty) {
    print_string(book_buf, "int parse_cjson_%s(cJSON* src, %s* dst);\n", ty->name, ty->name);
    print_string(book_buf, "int parse_json_%s(const char* src, unsigned long len, %s* dst);\n", ty->name, ty->name);
    return 2;
}
void gen_json_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro) {
    print_string(book_buf, "void dump_json_%s(%s* item, %s dst) {\n", ty->name, ty->name, dst_type);
    print_string(book_buf, "    BK_OFFSET_t offset = {0};\n");
    print_string(book_buf, "    %s(\"{\");\n", fmt_macro);
    for (size_t j = 0; j < ty->fields.len; ++j) {
        Field* f = ty->fields.items + j;
        switch (f->type.kind) {
        case CPRIMITIVE: {
            switch (f->type.type) {
            case CINT: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%d\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CUINT: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%u\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CLONG: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%ld\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CULONG: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%lu\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CFLOAT: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%f\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CBOOL: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%s\", item->%s ? \"true\" : \"false\");\n", fmt_macro, f->name, f->name);
            } break;
            case CSTRING: {
                // TODO: The generated code should escape item->field before printing it
                print_string(book_buf, "    %s(\"\\\"%s\\\":\\\"%%s\\\"\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CCHAR: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%c\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            default: abort();
            }
        } break;
        case CEXTERNAL: {
            print_string(book_buf, "    %s(\"\\\"%s\\\":\");\n", fmt_macro, f->name);
            print_string(book_buf, "    dump_json_%s(&item->%s, dst);\n", f->type.name, f->name);
        } break;
        default: abort();
        }
        if (j < ty->fields.len - 1) print_string(book_buf, "    %s(\",\");\n", fmt_macro);
    }
    print_string(book_buf, "    %s(\"}\");\n", fmt_macro);
    print_string(book_buf, "}\n");
}
void gen_json_parse_impl(String* book_buf, CCompound* ty) {
    print_string(book_buf, "int parse_cjson_%s(cJSON* src, %s* dst) {\n", ty->name, ty->name);
    for (size_t i = 0; i < ty->fields.len; ++i) {
        Field* f = ty->fields.items + i;
        print_string(book_buf, "    cJSON* %s_%s = cJSON_GetObjectItemCaseSensitive(src, \"%s\");\n", ty->name, f->name, f->name);
        // TODO: these early returns might leak memory, use something like `goto end`
        print_string(book_buf, "    if (!%s_%s) return 0;\n", ty->name, f->name);
        if (f->type.kind == CEXTERNAL) {
            // TODO: these early returns might leak memory, use something like `goto end`
            print_string(book_buf, "    if (!parse_cjson_%s(%s_%s, &dst->%s)) return 0;\n", f->type.name, ty->name, f->name, f->name);
        } else if (f->type.kind == CPRIMITIVE) {
            switch (f->type.type) {
            case CINT: case CUINT: {
                print_string(book_buf, "    if (cJSON_IsNumber(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valueint;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                // TODO: these early returns might leak memory, use something like `goto end`
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
            } break;
            case CLONG: case CULONG: case CFLOAT: {
                print_string(book_buf, "    if (cJSON_IsNumber(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valuedouble;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                // TODO: these early returns might leak memory, use something like `goto end`
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
            } break;
            case CCHAR: {
                print_string(book_buf, "    if (cJSON_IsString(%s_%s)) {\n", ty->name, f->name);
                // TODO: these early returns might leak memory, use something like `goto end`
                print_string(book_buf, "        if (!%s_%s->valuestring) { return 0; };\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = *%s_%s->valuestring;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                // TODO: these early returns might leak memory, use something like `goto end`
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
                    
            } break;
            case CBOOL: {
                print_string(book_buf, "    if (cJSON_IsBool(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valueint;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                // TODO: these early returns might leak memory, use something like `goto end`
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
                    
            } break;
            case CSTRING: {
                print_string(book_buf, "    if (cJSON_IsString(%s_%s)) {\n", ty->name, f->name);
                // TODO: these early returns might leak memory, use something like `goto end`
                print_string(book_buf, "        if (!%s_%s->valuestring) { return 0; };\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = strdup(%s_%s->valuestring);\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                // TODO: these early returns might leak memory, use something like `goto end`
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
            } break;
            }
        } else {
            abort();
        }
    }
    print_string(book_buf, "    return 1;\n");
    print_string(book_buf, "}\n");
    print_string(book_buf, "int parse_json_%s(const char* src, unsigned long len, %s* dst) {\n", ty->name, ty->name);
    print_string(book_buf, "    cJSON* json = cJSON_ParseWithLength(src, len);\n");
    print_string(book_buf, "    if (!json) return 0;\n");
    print_string(book_buf, "    int res = parse_cjson_%s(json, dst);\n", ty->name);
    print_string(book_buf, "    cJSON_Delete(json);\n");
    print_string(book_buf, "    return res;\n");
    print_string(book_buf, "}\n");
}

// Debug generation
size_t gen_debug_dump_decl(String* book_buf, CCompound* ty, const char* dst_type) {    
    print_string(book_buf, "void __indent_dump_debug_%s(%s* item, %s dst, int indent);\n", ty->name, ty->name, dst_type);
    print_string(book_buf, "void dump_debug_%s(%s* item, %s dst);\n", ty->name, ty->name, dst_type);
    return 2;
}
void gen_debug_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro) {
    print_string(book_buf, "void __indent_dump_debug_%s(%s* item, %s dst, int indent) {\n", ty->name, ty->name, dst_type);
    print_string(book_buf, "    BK_OFFSET_t offset = {0};\n");
    print_string(book_buf, "    %s(\"%s {\\n\");\n", fmt_macro, ty->name);
    for (size_t j = 0; j < ty->fields.len; ++j) {
        Field* f = ty->fields.items + j;
        switch (f->type.kind) {
        case CPRIMITIVE: {
            switch (f->type.type) {
            case CINT: {
                print_string(book_buf, "    %s(\"%%*s(int) %s: %%d\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CUINT: {
                print_string(book_buf, "    %s(\"%%*s(uint) %s: %%u\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CLONG: {
                print_string(book_buf, "    %s(\"%%*s(long) %s: %%ld\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CULONG: {
                print_string(book_buf, "    %s(\"%%*s(ulong) %s: %%lu\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CFLOAT: {
                print_string(book_buf, "    %s(\"%%*s(float) %s: %%f\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CBOOL: {
                print_string(book_buf, "    %s(\"%%*s(bool) %s: %%s\\n\", indent + 4, \"\", item->%s ? \"true\" : \"false\");\n", fmt_macro, f->name, f->name);
            } break;
            case CSTRING: {
                // TODO: The generated code should escape item->field before printing it
                print_string(book_buf, "    %s(\"%%*s(string) %s: %%s\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            case CCHAR: {
                print_string(book_buf, "    %s(\"%%*s(char) %s: %%c\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, f->name, f->name);
            } break;
            default: abort();
            }
        } break;
        case CEXTERNAL: {
            print_string(book_buf, "    %s(\"%%*s%s: \", indent + 4, \"\");\n", fmt_macro, f->name);
            print_string(book_buf, "    __indent_dump_debug_%s(&item->%s, dst, indent + 4);\n", f->type.name, f->name);
        } break;
        default: abort();
        }
    }
    print_string(book_buf, "    %s(\"%%*s}\\n\", indent, \"\");\n", fmt_macro);
    print_string(book_buf, "}\n");

    print_string(book_buf, "void dump_debug_%s(%s* item, %s dst) {\n", ty->name, ty->name, dst_type);
    print_string(book_buf, "    __indent_dump_debug_%s(item, dst, 0);\n", ty->name);
    print_string(book_buf, "}\n");
}

bool help_cmd(BkState* bk, int* i, int argc, char** argv) {
    (void)bk;

    if ((*i + 1) < argc) {
        char* arg = argv[*i + 1];
        if (strlen(arg) < 1) return false;
        if (arg[0] != '-') {
            bool found = false;
            for (size_t j = 0; j < commands_count; ++j) {
                if (strcmp(arg, commands[j].name) == 0) {
                    found = true;
                    printf("%s:\n    Usage: %s\n    Description: %s\n\n", commands[j].name, commands[j].usage, commands[j].desc);
                    break;
                }
            }
            return found;
        }
    }
    printf("[help start]\n\n");
    for (size_t j = 0; j < commands_count; ++j) {
        printf("%s: %s\n\n", commands[j].name, commands[j].desc);
    }
    printf("[help end]\n\n");

    return true;    
}

bool search_directory_cmd(BkState* bk, int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk->input_path = argv[*i];
        return true;
    }
    return false;
}

bool output_directory_cmd(BkState* bk, int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk->out_path = argv[*i];
        return true;
    }
    return false;
}

bool silent_cmd(BkState* bk, int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk->silent = true;
    return true;
}

bool derive_all_cmd(BkState* bk, int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk->derive_all = true;
    return true;
}

bool gen_impl_cmd(BkState* bk, int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk->gen_implementation_macro = argv[*i];
        return true;
    }
    return false;
}

bool gen_fmt_dst_cmd(BkState* bk, int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk->gen_fmt_dst_macro = argv[*i];
        return true;
    }
    return false;
}

bool gen_fmt_cmd(BkState* bk, int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk->gen_fmt_macro = argv[*i];
        return true;
    }
    return false;
}

bool disable_dump_cmd(BkState* bk, int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk->disable_dump_macro = argv[*i];
        return true;
    }
    return false;
}

bool disable_parse_cmd(BkState* bk, int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk->disable_parse_macro = argv[*i];
        return true;
    }
    return false;
}

bool offset_type_cmd(BkState* bk, int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk->offset_type_macro = argv[*i];
        return true;
    }
    return false;
}
