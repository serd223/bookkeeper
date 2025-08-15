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
#ifndef __BOOKKEEPER_GEN_C__
#define __BOOKKEEPER_GEN_C__

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

// #define BK_ENABLE_BK_CONF_GEN

#ifndef __BK_GEN_EXT_DEFINITIONS
#define __BK_GEN_EXT_DEFINITIONS

// This would typically be in drives.h, but the single file limitation strikes again
#define derive_bkconf(...)

typedef struct {
    char* output_mode;
    bool generics;
    bool silent;
    bool verbose;
    bool warn_unknown_attr;
    bool warn_no_include;
    bool warn_no_output;
    bool disable_dump;
    bool disable_parse;
    bool disabled_by_default;
    bool watch_mode;
    long watch_delay;
    char* gen_fmt_macro;
    char* gen_implementation_macro;
    char* gen_fmt_dst_macro;
    char* offset_type_macro;
    char* disable_macro_prefix;
    char* enable_macro_prefix;
    bool derive_all;
    char* include_dir;
    char* include_files; // This field is only used for loading include files from configs
    char* output_dir;
} BkConfig derive_bkconf();

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
    char* name; // CEXTERNAL
    CPrimitive type;  // CPIRIMITIVE
} CType;

typedef struct {
    CType* items;
    size_t len;
    size_t cap;
} CTypes;

typedef struct {
    char* name;
    char* tag;
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
    char* name;
} CCompound;

typedef struct {
    CCompound* items;
    size_t len;
    size_t cap;
} CCompounds;

typedef struct {
    size_t (*gen_dump_decl)(String* book_buf, CCompound* ty, const char* dst_type);
    size_t (*gen_parse_decl)(String* book_buf, CCompound* ty);
    void (*gen_dump_impl)(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);
    void (*gen_parse_impl)(String* book_buf, CCompound* ty);
    const char* derive_attr;
    const char* name;
} Schema;

typedef struct {
    Schema* items;
    size_t len;
    size_t cap;
} Schemas;

typedef struct {
    char* full;
    char* name;
    time_t sys_modif;
    time_t last_analyzed;
} Entry;

typedef struct {
    Entry* items;
    size_t len;
    size_t cap;
} Entries;

typedef struct {
    BkConfig conf;
    Entries entries;
    Schemas schemas;
} BkState;

static BkState bk = {0};
static char tmp_str[4096];
#define tfmt(...) (sprintf(tmp_str, __VA_ARGS__), tmp_str)
#define fmt(...) strdup(tfmt(__VA_ARGS__))

#define bk_log_loc(level, source, line, ...) do {\
    if (!bk.conf.silent) {\
        switch (level) {\
        case LOG_INFO: {\
            if (bk.conf.verbose) {\
                fprintf(stderr, "%s:%d: [INFO] ", source, line);\
                fprintf(stderr, __VA_ARGS__);\
            }\
        } break;\
        case LOG_WARN: {\
            if (bk.conf.verbose) {\
                fprintf(stderr, "%s:%d: [WARN] ", source, line);\
            } else {\
                fprintf(stderr, "[WARN] ");\
            }\
            fprintf(stderr, __VA_ARGS__);\
        } break;\
        case LOG_ERROR: {\
            if (bk.conf.verbose) {\
                fprintf(stderr, "%s:%d: [ERROR] ", source, line);\
            } else {\
                fprintf(stderr, "[ERROR] ");\
            }\
            fprintf(stderr, __VA_ARGS__);\
        } break;\
        default: abort();\
        }\
    }\
} while(0)

#define bk_log(level, ...) bk_log_loc(level, __FILE__, __LINE__, __VA_ARGS__)

typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} Log_Level;

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

#define __BK_API static

// General utility functions
__BK_API bool read_entire_file_loc(const char* file_name, String* dst, const char* source_file, int source_line);
#define read_entire_file(file_name, dst) read_entire_file_loc(file_name, dst, __FILE__, __LINE__)
__BK_API bool write_entire_file_loc(const char* file_name, String* src, const char* source_file, int source_line);
#define write_entire_file(file_name, src) write_entire_file_loc(file_name, src, __FILE__, __LINE__)
__BK_API unsigned long djb2(const char* s);
__BK_API bool entry_from_file(const char* file_name, Entry* out);

// Cleanup functions
__BK_API void free_ccompund(CCompound cc);
__BK_API void free_field(Field f);
__BK_API void free_ctype(CType ct);
__BK_API void free_entry(Entry e);

// Parsing C code
typedef enum {
    WORD_TOK,
    WORD_ID,
    WORD_END
} WordKind;

typedef struct {
    WordKind kind;
    union {
        int tok;
        const char* id;
    };
} Word;

#define ID(s) (Word){.kind = WORD_ID, .id=s}
#define TK(t) (Word){.kind = WORD_TOK, .tok=t}
#define WEND (Word){.kind = WORD_END}

__BK_API bool va_get_expect_ids(stb_lexer* lex, va_list args);
__BK_API bool va_get_expect_tokens(stb_lexer* lex, va_list args);
__BK_API bool va_get_expect_c(stb_lexer* lex, va_list args);
__BK_API bool peek_ids(stb_lexer* lex, ...);
__BK_API bool peek_tokens(stb_lexer* lex, ...);
__BK_API bool peek_c(stb_lexer* lex, ...);
__BK_API bool get_expect_tokens(stb_lexer* lex, ...);
__BK_API bool get_expect_ids(stb_lexer* lex,...);
bool get_expect_c(stb_lexer* lex,...); // Unused for now
__BK_API void analyze_file(Schemas schemas, String content, CCompounds* out, bool derive_all);

// Command line parsing
static char* parse_list(char** src, char sep, size_t* entry_len);

// Code generation
#define BK_DUMP_UPPER "DUMP"
#define BK_PARSE_UPPER "PARSE"
#define BK_DUMP_LOWER "dump"
#define BK_PARSE_LOWER "parse"


#define gen_def_guard(fntype)\
if (bk.conf.disabled_by_default) {\
    print_string(book_buf, "\n#if defined(%s"fntype")", bk.conf.enable_macro_prefix);\
    print_string(book_buf, " || defined(%s%s)", bk.conf.enable_macro_prefix, ty->name);\
    print_string(book_buf, " || defined(%s%s_"fntype")", bk.conf.enable_macro_prefix, ty->name);\
} else {\
    print_string(book_buf, "\n#ifndef %s"fntype"\n", bk.conf.disable_macro_prefix);\
    print_string(book_buf, "\n#ifndef %s%s\n", bk.conf.disable_macro_prefix, ty->name);\
    print_string(book_buf, "\n#ifndef %s%s_"fntype, bk.conf.disable_macro_prefix, ty->name);\
}\

#define gen_endif_guard(fntype)\
if (bk.conf.disabled_by_default) {\
    print_string(book_buf, "\n#endif // %s*\n", bk.conf.enable_macro_prefix);\
} else {\
    print_string(book_buf, "\n#endif // %s%s_"fntype"\n", bk.conf.disable_macro_prefix, ty->name);\
    print_string(book_buf, "\n#endif // %s%s\n", bk.conf.disable_macro_prefix, ty->name);\
    print_string(book_buf, "\n#endif // %s"fntype"\n", bk.conf.disable_macro_prefix);\
}\

#define gen_def_type_guard(fntype)\
if (bk.conf.disabled_by_default) {\
    gen_def_guard(fntype);\
    print_string(book_buf, " || defined(%s%s)", bk.conf.enable_macro_prefix, schema->name);\
    print_string(book_buf, "|| defined(%s%s_"fntype")", bk.conf.enable_macro_prefix, schema->name);\
    print_string(book_buf, "|| defined(%s%s_%s)", bk.conf.enable_macro_prefix, ty->name, schema->name);\
    print_string(book_buf, "|| defined(%s%s_%s_"fntype")\n", bk.conf.enable_macro_prefix, ty->name, schema->name);\
} else {\
    print_string(book_buf, "\n#ifndef %s%s\n", bk.conf.disable_macro_prefix, schema->name);\
    print_string(book_buf, "\n#ifndef %s%s_"fntype"\n", bk.conf.disable_macro_prefix, schema->name);\
    print_string(book_buf, "\n#ifndef %s%s_%s\n", bk.conf.disable_macro_prefix, ty->name, schema->name);\
    print_string(book_buf, "\n#ifndef %s%s_%s_"fntype"\n", bk.conf.disable_macro_prefix, ty->name, schema->name);\
}\

#define gen_endif_type_guard(fntype)\
if (bk.conf.disabled_by_default) {\
    print_string(book_buf, "\n#endif // %s*\n", bk.conf.enable_macro_prefix);\
} else {\
    print_string(book_buf, "\n#endif // %s%s_%s_"fntype"\n", bk.conf.disable_macro_prefix, ty->name, schema->name);\
    print_string(book_buf, "\n#endif // %s%s_%s\n", bk.conf.disable_macro_prefix, ty->name, schema->name);\
    print_string(book_buf, "\n#endif // %s%s_"fntype"\n", bk.conf.disable_macro_prefix, schema->name);\
    print_string(book_buf, "\n#endif // %s%s\n", bk.conf.disable_macro_prefix, schema->name);\
}\

size_t gen_dump_decl(Schemas schemas, String* book_buf, CCompound* ty, const char* dst_type);
size_t gen_parse_decl(Schemas schemas, String* book_buf, CCompound* ty);
void gen_dump_impl(Schemas schemas, String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);
void gen_parse_impl(Schemas schemas, String* book_buf, CCompound* ty);

// JSON generation
size_t gen_json_dump_decl(String* book_buf, CCompound* ty, const char* dst_type);
size_t gen_json_parse_decl(String* book_buf, CCompound* ty);
void gen_json_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);
void gen_json_parse_impl(String* book_buf, CCompound* ty);

// Debug generation
size_t gen_debug_dump_decl(String* book_buf, CCompound* ty, const char* dst_type);
void gen_debug_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);

#ifdef BK_ENABLE_BK_CONF_GEN
// bkconf generation
size_t gen_bkconf_parse_decl(String* book_buf, CCompound* ty);
void gen_bkconf_parse_impl(String* book_buf, CCompound* ty);
#endif // BK_ENABLE_BK_CONF_GEN

// Generated code, generated by enabling `BK_ENABLE_BK_CONF_GEN`
int parse_bkconf_BkConfig(const char* src, unsigned long len, BkConfig* dst);

typedef enum {
    O_MIRROR,
    O_DIR,
    // O_FILE // TODO: implement optional single-file output
} OutputMode;

// Command line arguments
typedef struct {
    char* name;
    char* flag;
    char* usage;
    char* desc;
    bool (*exec_c)(int* i, int argc, char** argv);
} Command;

// If only we had closures in C...
bool help_cmd(int* i, int argc, char** argv);
bool config_path_cmd(int* i, int argc, char** argv);
bool output_mode_cmd(int* i, int argc, char** argv);
bool gen_ext_cmd(int* i, int argc, char** argv);
bool generics_cmd(int* i, int argc, char** argv);
bool watch_cmd(int* i, int argc, char** argv);
bool watch_delay_cmd(int* i, int argc, char** argv);
bool include_file_cmd(int* i, int argc, char** argv);
bool include_directory_cmd(int* i, int argc, char** argv);
bool output_directory_cmd(int* i, int argc, char** argv);
bool schemas_cmd(int* i, int argc, char** argv);
bool silent_cmd(int* i, int argc, char** argv);
bool verbose_cmd(int* i, int argc, char** argv);
bool enable_warn_cmd(int* i, int argc, char** argv);
bool disable_warn_cmd(int* i, int argc, char** argv);
bool derive_all_cmd(int* i, int argc, char** argv);
bool disable_dump_cmd(int* i, int argc, char** argv);
bool disable_parse_cmd(int* i, int argc, char** argv);
bool disabled_cmd(int* i, int argc, char** argv);
bool gen_impl_cmd(int* i, int argc, char** argv);
bool gen_fmt_dst_cmd(int* i, int argc, char** argv);
bool gen_fmt_cmd(int* i, int argc, char** argv);
bool offset_type_cmd(int* i, int argc, char** argv);
bool disable_prefix_cmd(int* i, int argc, char** argv);
bool enable_prefix_cmd(int* i, int argc, char** argv);

#define exec_cmd(cmd)\
((cmd)->exec_c ? ((cmd)->exec_c(&i, argc, argv) ? true : (bk_printf("Usage of '%s': %s\n", (cmd)->name, (cmd)->usage), false)) : false)

#define WARN_NO_INCLUDE "no-include"
#define WARN_NO_OUTPUT "no-output"
#define WARN_UNKNOWN_ATTR "unknown-attr"
#define WARN_LIST WARN_NO_INCLUDE"|"WARN_NO_OUTPUT"|"WARN_UNKNOWN_ATTR

static Command commands[] = {
    {
        .name = "help",
        .flag = "-h",
        .usage = "-h <command (optional)>",
        .desc = "Prints a list of all commands or information about the provided command",
        .exec_c = help_cmd
    },
    {
        .name = "config-path",
        .flag = "--config-path",
        .usage = "--config-path <file>",
        .desc = "Changes the path that will be used to load the configuration file (default value is './bk.conf.conf')",
        .exec_c = config_path_cmd
    },
    {
        .name = "output-mode",
        .flag = "-om",
        .usage = "-om <mirror|dir>",
        .desc = "Sets the preffered output mode. `mirror` puts generated files next to the files they were generated from. `dir` puts all generated files in the specified `output-directory`. (`derives.h` is always placed inside `output-directory`)",
        .exec_c = output_mode_cmd
    },
    {
        .name = "gen-ext",
        .flag = "--gen-ext",
        .usage = "--gen-ext <bk-source> <out-file>",
        .desc = "Generates the extension header from `bk-source` (bk.c) that contains the definitions that should be included inside static schema extensions.",
        .exec_c = gen_ext_cmd
    },
    {
        .name = "generics",
        .flag = "--generics",
        .usage = "--generics",
        .desc = "Generates generic macros for dump/parse functions. These macros rely on schemas respecting the `dump/parse_$schema$_$type$` standard. The generic macros will be placed inside `output-directory/generics.h`",
        .exec_c = generics_cmd
    },
    {
        .name = "watch",
        .flag = "-w",
        .usage = "-w",
        .desc = "Enables watch mode that constantly analyzes recently modified files with a `watch-delay` second delay. (Exit with `CTRL-C`)",
        .exec_c = watch_cmd
    },
    {
        .name = "watch-delay",
        .flag = "--watch-delay",
        .usage = "--watch-delay <integer>",
        .desc = "Sets `watch-delay` option, for more information see `watch`.",
        .exec_c = watch_delay_cmd
    },
    {
        .name = "include-file",
        .flag = "-i",
        .usage = "-i <file>",
        .desc = "The included file will be analyzed regardless of its extension",
        .exec_c = include_file_cmd
    },
    {
        .name = "include-directory",
        .flag = "-I",
        .usage = "-I <dir>",
        .desc = "The provided directory will be searched for '.c' or '.h' files to analyze",
        .exec_c = include_directory_cmd
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
        .flag = "-o",
        .usage = "-o <dir>",
        .desc = "All generated files will be placed inside the provided directory",
        .exec_c = output_directory_cmd
    },
    {
        .name = "schemas",
        .flag = "--schemas",
        .usage = "--schemas",
        .desc = "Displays a list of loaded schemas",
        .exec_c = schemas_cmd
    },
    {
        .name = "silent",
        .flag = "--silent",
        .usage = "--silent",
        .desc = "Disables all terminal output",
        .exec_c = silent_cmd
    },
    {
        .name = "verbose",
        .flag = "-v",
        .usage = "-v",
        .desc = "Enables verbose terminal output",
        .exec_c = verbose_cmd
    },
    {
        .name = "enable-warning",
        .flag = "-W",
        .usage = "-W <"WARN_LIST">",
        .desc = "Enables the specified warning",
        .exec_c = enable_warn_cmd
    },
    {
        .name = "disable-warning",
        .flag = "-dW",
        .usage = "-dW <"WARN_LIST">",
        .desc = "Disables the specified warning",
        .exec_c = disable_warn_cmd
    },
    {
        .name = "derive-all",
        .flag = "--derive-all",
        .usage = "--derive-all",
        .desc = "Derives all possible schemas for all analyzed structs",
        .exec_c = derive_all_cmd
    },
    {
        .name = "disable-dump",
        .flag = "--disable-dump",
        .usage = "--disable-dump",
        .desc = "Disables the generation of `dump` functions",
        .exec_c = disable_dump_cmd
    },
    {
        .name = "disable-parse",
        .flag = "--disable-parse",
        .usage = "--disable-parse",
        .desc = "Disables the generation of `parse` functions",
        .exec_c = disable_parse_cmd
    },
    {
        .name = "disabled",
        .flag = "--disabled",
        .usage = "--disabled",
        .desc = "Disables all functionality by default. They can be enabled gradually in code with `ENABLE` macros.",
        .exec_c = disabled_cmd
    },
    {
        .name = "gen-implementation",
        .flag = "--gen-implementation",
        .usage = "--gen-implementation <name>",
        .desc = "Sets the macro that will be used in the generated code to control enabling implementation (`BK_IMPLEMENTATION`)",
        .exec_c = gen_impl_cmd
    },
    {
        .name = "gen-fmt-dst",
        .flag = "--gen-fmt-dst",
        .usage = "--gen-fmt-dst <name>",
        .desc = "Sets the macro that will be used in the generated code to control the type of `dst` in `dump` functions (`BK_FMT_DST_t`)",
        .exec_c = gen_fmt_dst_cmd
    },
    {
        .name = "gen-fmt",
        .flag = "--gen-fmt",
        .usage = "--gen-fmt <name>",
        .desc = "Sets the macro that will be used in the generated `dump` functions to output with `printf` style arguments (`BK_FMT`)",
        .exec_c = gen_fmt_cmd
    },
    {
        .name = "offset-type",
        .flag = "--offset-type",
        .usage = "--offset-type <name>",
        .desc = "Sets the macro that will be used in the generated code to control the type of the `offset` variable inside `dump` functions (`BK_OFFSET_t`)",
        .exec_c = offset_type_cmd
    },
    {
        .name = "disable-prefix",
        .flag = "--disable-prefix",
        .usage = "--disable-prefix <name>",
        .desc = "Sets the prefix of the generated macros that disable specific stuff, like `$prefix$$type$_$schema$` (`BK_DISABLE_`)",
        .exec_c = disable_prefix_cmd
    },
    {
        .name = "enable-prefix",
        .flag = "--enable-prefix",
        .usage = "--enable-prefix <name>",
        .desc = "Sets the prefix of the generated macros that enable specific stuff, like `$prefix$$type$_$schema$` (`BK_ENABLE_`)",
        .exec_c = enable_prefix_cmd
    },
};
const static size_t commands_count = sizeof commands / sizeof *commands;

#define bk_printf(...) (bk.conf.silent ? 0 : printf(__VA_ARGS__))

static char* config_path = "./.bk.conf";
#define BK_FILE_EXT ".bk.h"
#define BK_FILE_EXT_LEN 5

#define ret_clean(i)\
do {\
    ret_val = i;\
    goto __bk_cleanup;\
} while(0)

#ifdef BK_RENAME_MAIN
#define __BK_DEFINE_FN(name) int name(int argc, char** argv)
__BK_DEFINE_FN(BK_RENAME_MAIN) {
#undef __BK_DEFINE_FN
#else
int main(int argc, char** argv) {
#endif // BK_RENAME_MAIN

    // Declaring resources that will be cleaned up here before declaring
    // `ret_val` to avoid situations where we try to cleanup these resources
    // before they were *declared* by goto'ing to `__bk_cleanup` inside `ret_clean`
    CCompounds all_types = {0};
    CCompounds types = {0};
    String file_buf = {0};
    
    int ret_val = 0;

    Schema json = {
        .gen_dump_decl = gen_json_dump_decl, 
        .gen_parse_decl = gen_json_parse_decl, 
        .gen_dump_impl = gen_json_dump_impl, 
        .gen_parse_impl = gen_json_parse_impl, 
        .derive_attr = "derive_json",
        .name = "json"
    };
    Schema debug = {
        .gen_dump_decl = gen_debug_dump_decl, 
        .gen_parse_decl = NULL, 
        .gen_dump_impl = gen_debug_dump_impl, 
        .gen_parse_impl = NULL, 
        .derive_attr = "derive_debug",
        .name = "debug"
    };
    push_da(&bk.schemas, json);
    push_da(&bk.schemas, debug);

    #ifdef BK_ENABLE_BK_CONF_GEN
    Schema bkconf = {  
        .gen_dump_decl = NULL, 
        .gen_parse_decl = gen_bkconf_parse_decl, 
        .gen_dump_impl = NULL, 
        .gen_parse_impl = gen_bkconf_parse_impl, 
        .derive_attr = "derive_bkconf",
        .name = "bkconf"
    };
    push_da(&bk.schemas, bkconf);
    #endif // BK_ENABLE_BK_CONF_GEN

    // For schema extensions
    #ifdef BK_ADD_SCHEMAS
    do {
        BK_ADD_SCHEMAS((bk.schemas))
    } while(0);
    #endif

    bk.conf.output_mode = "mirror";
    bk.conf.gen_fmt_macro = "BK_FMT";
    bk.conf.gen_implementation_macro = "BK_IMPLEMENTATION";
    bk.conf.gen_fmt_dst_macro = "BK_FMT_DST_t";
    // bk.conf.disable_dump_macro = "BK_DISABLE_DUMP";
    // bk.conf.disable_parse_macro = "BK_DISABLE_PARSE";
    bk.conf.offset_type_macro = "BK_OFFSET_t";
    bk.conf.disable_macro_prefix = "BK_DISABLE_";
    bk.conf.enable_macro_prefix = "BK_ENABLE_";

    bk.conf.generics = false;
    bk.conf.silent = false;
    bk.conf.verbose = false;
    bk.conf.warn_no_include = false;
    bk.conf.warn_no_output = true;
    bk.conf.warn_unknown_attr = true;
    bk.conf.derive_all = false;
    bk.conf.watch_mode = false;
    bk.conf.watch_delay = 5;
    bk.conf.include_dir = NULL;
    bk.conf.output_dir = NULL;

    #ifdef DEBUG
    bk.conf.silent = true;
    #endif // DEBUG

    int config_cmd_index = -1;
    for (size_t i = 0; i < commands_count; ++i) {
        if (strcmp(commands[i].name, "config-path") == 0) {
            config_cmd_index = (int)i;
        }
    }
    if (config_cmd_index < 0) abort();

    bool custom_config = false;
    // NOTE: Assumes that `argv` lives long enough
    for (int i = 1; i < argc; ++i) {
        if (strcmp(commands[config_cmd_index].flag, argv[i]) == 0) {
            if (!exec_cmd(&commands[config_cmd_index])) return 1;
            custom_config = true;
            break;
        }
    }

    bool found_config = false;
    {
        FILE* conf_file = fopen(config_path, "rb");
        if (conf_file) {
            fclose(conf_file);
            String conf_contents = {0};
            if (read_entire_file(config_path, &conf_contents)) {
                found_config = true;
                parse_bkconf_BkConfig(conf_contents.items, conf_contents.len, &bk.conf);
            }
        } else if (custom_config) {
            bk_log(LOG_ERROR, "Couldn't open file '%s': %s\n", config_path, strerror(errno));
            ret_clean(1);
        }
    }

    if (argc <= 1 && !found_config) {
        bk_printf("Basic usage: %s -I <include-directory> -o <output-directory>\n", argv[0]);
        bk_printf("Use `-h` to print all available commands, `-h <command-name>` to see that command's usage.\n");
        ret_clean(0);
    }

    // NOTE: Assumes that `argv` lives long enough
    for (int i = 1; i < argc; ++i) {
        for (size_t j = 0; j < commands_count; ++j) {
            if (strcmp(commands[j].flag, argv[i]) == 0) {
                if (!exec_cmd(&commands[j])) {
                    ret_clean(1);
                }
            }
        }
    }
   
    if (bk.conf.include_files != NULL) {
        // size_t name_start = 0;
        size_t list_len = strlen(bk.conf.include_files);
        if (list_len == 0) {
            bk_log(LOG_ERROR, "Include file list in '%s' is empty.\n", config_path);
            ret_clean(1);
        }
        if (bk.conf.include_files[0] == ',') {
            bk_log(LOG_ERROR, "Include file list in '%s' starts with comma (',')\n", config_path);
            ret_clean(1);
        }
        char* cursor = bk.conf.include_files;
        size_t entry_len = 0;

        for (char* ent; (ent = parse_list(&cursor, ',', &entry_len));) {
            Entry e = {0};
            if (!entry_from_file(tfmt("%.*s", (int)entry_len, ent), &e)) {
                ret_clean(1);
            }
            push_da(&bk.entries, e);
        }
    }

    if (bk.conf.include_dir == NULL && bk.entries.len == 0) {
        if (bk.conf.warn_no_include) bk_log(LOG_WARN, "No files were included\n");
    }
    if (bk.conf.output_dir == NULL) {
        if (bk.conf.warn_no_output) bk_log(LOG_WARN, "No output path set\n");
    }
    
    {
        if (bk.conf.include_dir != NULL) {
            size_t in_len = strlen(bk.conf.include_dir);
            if (bk.conf.include_dir[in_len - 1] == '/') bk.conf.include_dir[in_len - 1] = 0;
        }
        if (bk.conf.output_dir != NULL) {
            size_t out_len = strlen(bk.conf.output_dir);
            if (bk.conf.output_dir[out_len - 1] == '/') bk.conf.output_dir[out_len - 1] = 0;
        }
    }

    OutputMode output_mode;
    if (strcmp(bk.conf.output_mode, "mirror") == 0) {
        output_mode = O_MIRROR;
    } else if (strcmp(bk.conf.output_mode, "dir") == 0) {
        output_mode = O_DIR;
    } else {
        bk_printf("Unknown output mode '%s', exiting...\n", bk.conf.output_mode);
        ret_clean(1);
    }

    String book_buf = {0};
    if (bk.conf.output_dir != NULL) {
        print_string(&book_buf, "#ifndef __DERIVES_H__\n");
        print_string(&book_buf, "#define __DERIVES_H__\n");
        print_string(&book_buf, "#define tag(s)\n");
        print_string(&book_buf, "#define derive_all(...)\n");
        for (size_t i = 0; i < bk.schemas.len; ++i) {
            print_string(&book_buf, "#define %s(...)\n", bk.schemas.items[i].derive_attr);
        }
        print_string(&book_buf, "#endif // __DERIVES_H__\n");
        write_entire_file(tfmt("%s/derives.h", bk.conf.output_dir), &book_buf);
    }

    if (bk.conf.include_dir != NULL) {
        DIR* input_dir = opendir(bk.conf.include_dir); // TODO: this is our main POSIX-dependent piece of code, perhaps write a cross-platform wrapper 
        if (!input_dir) {
            bk_log(LOG_ERROR, "Couldn't open directory '%s': %s\n", bk.conf.include_dir, strerror(errno));
            ret_clean(1);
        }
        for (struct dirent* ent = readdir(input_dir); ent; ent = readdir(input_dir)) {
            if (ent->d_type == DT_REG) {
                size_t name_len = strlen(ent->d_name);
                if (name_len > BK_FILE_EXT_LEN && strcmp(ent->d_name + name_len - BK_FILE_EXT_LEN, BK_FILE_EXT) == 0) continue;
                if (name_len < 2) continue;
                if (
                    strcmp(ent->d_name + name_len - 2, ".c") == 0 ||
                    strcmp(ent->d_name + name_len - 2, ".h") == 0
                ) {
                    char* in_file = fmt("%s/%s", bk.conf.include_dir, ent->d_name); // alloc
                    struct stat s = {0};
                    stat(in_file, &s);

                    Entry e = {
                        .full = in_file,
                        .name = strdup(ent->d_name), // alloc
                        .sys_modif = s.st_mtim.tv_sec,
                        .last_analyzed = 0
                    };
                    push_da(&bk.entries, e);
                }
            }
        }
        closedir(input_dir);
    }

    if (bk.entries.len <= 0 || (output_mode == O_DIR && bk.conf.output_dir == NULL)) ret_clean(0);

    size_t file_idx = 0;
    book_buf.len = 0;
    time_t current_iter = 0;
    time_t t = 0;
    // TODO: This is a very basic implementation of a watch mode and causes CPUs to spin for no reason
    // might want to use something like `poll` to detect file events?
    do {
        t = time(NULL);
        if (t - current_iter < (time_t)bk.conf.watch_delay) continue;
        current_iter = t;
        for (size_t i = 0; i < all_types.len; ++i) free_ccompund(all_types.items[i]);
        all_types.len = 0;
        for (size_t e_i = 0; e_i < bk.entries.len; ++e_i) {
            Entry* in_file = bk.entries.items + e_i;
            {
                struct stat s = {0};
                stat(in_file->full, &s);
                in_file->sys_modif = s.st_mtim.tv_sec;
            }
            if (in_file->sys_modif > in_file->last_analyzed) {
                unsigned long in_hash = djb2(in_file->full);
                bk_log(LOG_INFO, "Analyzing file: %s\n", in_file->name);
                file_buf.len = 0;
                if (!read_entire_file(in_file->full, &file_buf)) continue;

                for (size_t i = 0; i < types.len; ++i) {
                    push_da(&all_types, types.items[i]);
                };
                types.len = 0;
                analyze_file(bk.schemas, file_buf, &types, bk.conf.derive_all);
                bk_log(LOG_INFO, "Analayzed %lu type(s).\n", types.len);
                book_buf.len = 0;
                current_iter = time(NULL);
                in_file->last_analyzed = current_iter;
                if (types.len > 0) {
                    print_string(&book_buf, "#ifndef __BK_%lu_%lu_H__ // Generated from: %s\n", in_hash, file_idx, in_file->full);
                    print_string(&book_buf, "#define __BK_%lu_%lu_H__\n", in_hash, file_idx);
                    print_string(&book_buf, "#ifndef %s\n", bk.conf.gen_fmt_dst_macro);
                    print_string(&book_buf, "#define %s FILE*\n", bk.conf.gen_fmt_dst_macro);
                    print_string(&book_buf, "#endif // %s\n", bk.conf.gen_fmt_dst_macro);
                    print_string(&book_buf, "#ifndef %s\n", bk.conf.gen_fmt_macro);
                    print_string(&book_buf, "#define %s(...) offset += fprintf(dst, __VA_ARGS__)\n", bk.conf.gen_fmt_macro);
                    print_string(&book_buf, "#endif // %s\n", bk.conf.gen_fmt_macro);
                    print_string(&book_buf, "#ifndef %s\n", bk.conf.offset_type_macro);
                    print_string(&book_buf, "#define %s size_t\n", bk.conf.offset_type_macro);
                    print_string(&book_buf, "#endif // %s\n", bk.conf.offset_type_macro);
                    size_t num_decls = 0;
                    for (size_t i = 0; i < types.len; ++i) {
                        if (!bk.conf.disable_dump) {
                            num_decls += gen_dump_decl(bk.schemas, &book_buf, types.items + i, bk.conf.gen_fmt_dst_macro);
                        }
                        if (!bk.conf.disable_parse) {
                            num_decls += gen_parse_decl(bk.schemas, &book_buf, types.items + i);
                        }
                    }
                    if (num_decls > 0) {
                        print_string(&book_buf, "\n#ifdef %s\n", bk.conf.gen_implementation_macro);
                        for (size_t i = 0; i < types.len; ++i) {
                            if (!bk.conf.disable_dump) {
                                gen_dump_impl(bk.schemas, &book_buf, types.items + i, bk.conf.gen_fmt_dst_macro, bk.conf.gen_fmt_macro);
                            }
                            if (!bk.conf.disable_parse) {
                                gen_parse_impl(bk.schemas, &book_buf, types.items + i);
                            }
                            print_string(&book_buf, "\n#define ___BK_INCLUDE_TYPE_%s\n", types.items[i].name);
                        }
                        print_string(&book_buf, "\n#endif // %s\n", bk.conf.gen_implementation_macro);

                        push_da(&book_buf, '\n');
                        print_string(&book_buf, "#endif // __BK_%lu_%lu_H__\n", in_hash, file_idx);
                        char* out_file = NULL;
                        switch (output_mode) {
                        case O_MIRROR: {
                            out_file = tfmt("%s"BK_FILE_EXT, in_file->full);
                        } break;
                        case O_DIR: {
                            out_file = tfmt("%s/%s"BK_FILE_EXT, bk.conf.output_dir, in_file->name);
                        } break;
                        }
                        if (out_file) write_entire_file(out_file, &book_buf);
                        file_idx += 1; // increment index counter even if write_entire_file errors just to be safe
                    }
                }
            }
        }
        if (bk.conf.generics) {
            book_buf.len = 0;
            print_string(&book_buf, "#ifndef __GENERICS_H__\n");
            print_string(&book_buf, "#define __GENERICS_H__\n");
            for (size_t i = 0; i < all_types.len; ++i) {
                const char* t_name = all_types.items[i].name;
                print_string(&book_buf, "#ifdef ___BK_INCLUDE_TYPE_%s\n", t_name);
                print_string(&book_buf, "#define ___BK_IF_TYPE_%s(x) x,\n", t_name);
                print_string(&book_buf, "#else // ___BK_INCLUDE_TYPE_%s\n", t_name);
                print_string(&book_buf, "#define ___BK_IF_TYPE_%s(x)\n", t_name);
                print_string(&book_buf, "#endif // ___BK_INCLUDE_TYPE_%s\n", t_name);
            }
            for (size_t i = 0; i < bk.schemas.len; ++i) {
                const char* s_name = bk.schemas.items[i].name;

                // dump guard start
                if (bk.conf.disabled_by_default) {
                    print_string(&book_buf, "#if defined(%s"BK_DUMP_UPPER") || defined(%s%s) || defined(%s%s_"BK_DUMP_UPPER")\n",
                                 bk.conf.enable_macro_prefix,
                                 bk.conf.enable_macro_prefix,
                                 s_name,
                                 bk.conf.enable_macro_prefix,
                                 s_name
                    );
                } else {
                    print_string(&book_buf, "#ifndef %s"BK_DUMP_UPPER"\n", bk.conf.disable_macro_prefix);
                    print_string(&book_buf, "#ifndef %s%s\n", bk.conf.disable_macro_prefix, s_name);
                    print_string(&book_buf, "#ifndef %s%s_"BK_DUMP_UPPER"\n", bk.conf.disable_macro_prefix, s_name);
                }

                // dump code
                print_string(&book_buf, "#define ___BK_GENERIC_"BK_DUMP_UPPER"_%s_CASES\\\n", s_name);
                for (size_t j = 0; j < all_types.len; ++j) {
                    const char* t_name = all_types.items[j].name;
                    print_string(&book_buf, "    ___BK_IF_TYPE_%s(%s*: "BK_DUMP_LOWER"_%s_%s)\\\n", t_name, t_name, s_name, t_name);
                }
                print_string(&book_buf, "\n#define "BK_DUMP_LOWER"_%s(item, dst)\\\n", s_name);
                print_string(&book_buf, "_Generic((item), ___BK_GENERIC_"BK_DUMP_UPPER"_%s_CASES default: NULL)((item), (dst))\n", s_name);

                // dump guard end
                if (bk.conf.disabled_by_default) {
                    print_string(&book_buf, "#endif // defined(%s"BK_DUMP_UPPER") || defined(%s%s) || defined(%s%s_"BK_DUMP_UPPER")\n",
                                 bk.conf.enable_macro_prefix,
                                 bk.conf.enable_macro_prefix,
                                 s_name,
                                 bk.conf.enable_macro_prefix,
                                 s_name
                    );
                } else {
                    print_string(&book_buf, "#endif // %s%s_"BK_DUMP_UPPER"\n", bk.conf.disable_macro_prefix, s_name);
                    print_string(&book_buf, "#endif // %s%s\n", bk.conf.disable_macro_prefix, s_name);
                    print_string(&book_buf, "#endif // %s"BK_DUMP_UPPER"\n", bk.conf.disable_macro_prefix);
                }

                // parse guard start
                if (bk.conf.disabled_by_default) {
                    print_string(&book_buf, "#if defined(%s"BK_PARSE_UPPER") || defined(%s%s) || defined(%s%s_"BK_PARSE_UPPER")\n",
                                 bk.conf.enable_macro_prefix,
                                 bk.conf.enable_macro_prefix,
                                 s_name,
                                 bk.conf.enable_macro_prefix,
                                 s_name
                    );
                } else {
                    print_string(&book_buf, "#ifndef %s"BK_PARSE_UPPER"\n", bk.conf.disable_macro_prefix);
                    print_string(&book_buf, "#ifndef %s%s\n", bk.conf.disable_macro_prefix, s_name);
                    print_string(&book_buf, "#ifndef %s%s_"BK_PARSE_UPPER"\n", bk.conf.disable_macro_prefix, s_name);
                }

                // parse code
                print_string(&book_buf, "#define ___BK_GENERIC_"BK_PARSE_UPPER"_%s_CASES\\\n", s_name);
                for (size_t j = 0; j < all_types.len; ++j) {
                    const char* t_name = all_types.items[j].name;
                    print_string(&book_buf, "    ___BK_IF_TYPE_%s(%s*: "BK_PARSE_LOWER"_%s_%s)\\\n", t_name, t_name, s_name, t_name);
                }
                print_string(&book_buf, "\n#define "BK_PARSE_LOWER"_%s(src, len, dst)\\\n", s_name);
                print_string(&book_buf, "_Generic((dst), ___BK_GENERIC_"BK_PARSE_UPPER"_%s_CASES default: NULL)((src), (len), (dst))\n", s_name);

                // parse guard end
                if (bk.conf.disabled_by_default) {
                    print_string(&book_buf, "#endif // defined(%s"BK_PARSE_UPPER") || defined(%s%s) || defined(%s%s_"BK_PARSE_UPPER")\n",
                                 bk.conf.enable_macro_prefix,
                                 bk.conf.enable_macro_prefix,
                                 s_name,
                                 bk.conf.enable_macro_prefix,
                                 s_name
                    );
                } else {
                    print_string(&book_buf, "#endif // %s%s_"BK_PARSE_UPPER"\n", bk.conf.disable_macro_prefix, s_name);
                    print_string(&book_buf, "#endif // %s%s\n", bk.conf.disable_macro_prefix, s_name);
                    print_string(&book_buf, "#endif // %s"BK_PARSE_UPPER"\n", bk.conf.disable_macro_prefix);
                }

            }
            print_string(&book_buf, "#endif // __GENERICS_H__\n");
            write_entire_file(tfmt("%s/generics.h", bk.conf.output_dir), &book_buf);
        }
    } while(bk.conf.watch_mode);

    __bk_cleanup:
    if (types.items != NULL) {
        for (size_t i = 0; i < types.len; ++i) free_ccompund(types.items[i]);
        free(types.items);
    }

    if (all_types.items != NULL) {
        for (size_t i = 0; i < all_types.len; ++i) free_ccompund(all_types.items[i]);
        free(all_types.items);
    }

    if (file_buf.items != NULL) free(file_buf.items);
    if (bk.schemas.items != NULL) free(bk.schemas.items);

    if (bk.entries.items != NULL) {
        for (size_t i = 0; i < bk.entries.len; ++i) free_entry(bk.entries.items[i]);
        free(bk.entries.items);
    }
    
    return ret_val;
}

__BK_API bool read_entire_file_loc(const char* file_name, String* dst, const char* source_file, int source_line) {
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

__BK_API bool write_entire_file_loc(const char* file_name, String* src, const char* source_file, int source_line) {
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

__BK_API unsigned long djb2(const char* s) {
    unsigned long hash = 5381;
    for (char c; (c = *s++);) hash = ((hash << 5) + hash) + (unsigned long) c;
    return hash;
}

__BK_API bool entry_from_file(const char* file_name, Entry* out) {
    char* real = realpath(file_name, NULL); // alloc
    if (real == NULL) {
        bk_log(LOG_ERROR, "File '%s' doesn't exist: %s\n", file_name, strerror(errno));
        return false;
    }
    struct stat s = {0};
    stat(real, &s);
    out->full = real;
    out->name = strdup(file_name); // alloc
    out->sys_modif = s.st_mtim.tv_sec;
    out->last_analyzed = 0;

    return true;
}

// Cleanup functions
__BK_API void free_ccompund(CCompound cc) {        
    for (size_t i = 0; i < cc.fields.len; ++i) {
        free_field(cc.fields.items[i]);
    }
}

__BK_API void free_field(Field f) {
    free(f.name);
    free(f.tag);
    free_ctype(f.type);
}

__BK_API void free_ctype(CType ct) {
    if (ct.kind == CEXTERNAL) free(ct.name);
}

__BK_API void free_entry(Entry e) {
    free(e.full);
    free(e.name);
}

__BK_API bool va_get_expect_ids(stb_lexer* lex, va_list args) {
    const char* expected = va_arg(args, const char*);
    for (; expected != NULL; expected = va_arg(args, const char*)) {
        stb_c_lexer_get_token(lex);
        if (lex->token != CLEX_id)              return false;
        if (strcmp(lex->string, expected) != 0) return false;
    }
    return true;
}

__BK_API bool va_get_expect_tokens(stb_lexer* lex, va_list args) {
    int expected = va_arg(args, int);
    for (; expected > -1 ; expected = va_arg(args, int)) {
        stb_c_lexer_get_token(lex);
        if (lex->token != expected) return false;
    }
    return true;
}

__BK_API bool va_get_expect_c(stb_lexer* lex, va_list args) {
    Word expected = va_arg(args, Word);
    for(; expected.kind != WORD_END; expected = va_arg(args, Word)) {
        stb_c_lexer_get_token(lex);
        switch (expected.kind) {
        case WORD_TOK: {
            if (lex->token != expected.tok) return false;
        } break;
        case WORD_ID: {
            if (lex->token != CLEX_id) return false;
            if (strcmp(lex->string, expected.id) != 0) return false;
        } break;
        case WORD_END: {
            abort(); // unreachable
        } break;
        }
    }
    return true;    
}

/// Arguments should be of type `const char*` and end with NULL
__BK_API bool peek_ids(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    char* pp = lex->parse_point;
    bool res = va_get_expect_ids(lex, args);
    lex->parse_point = pp;
    va_end(args);
    return res;
}

/// Arguments should be of type `int` and end with -1
__BK_API bool peek_tokens(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    char* pp = lex->parse_point;
    bool res = va_get_expect_tokens(lex, args);
    lex->parse_point = pp;
    va_end(args);
    return res;
}

/// Arguments should be of type `Word` and end with WEND
__BK_API bool peek_c(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    char* pp = lex->parse_point;
    bool res = va_get_expect_c(lex, args);
    lex->parse_point = pp;
    va_end(args);
    return res;
}

/// Arguments should be of type `int` and end with -1
__BK_API bool get_expect_tokens(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    bool res = va_get_expect_tokens(lex, args);
    va_end(args);
    return res;
}

/// Arguments should be of type `const char*` and end with NULL
__BK_API bool get_expect_ids(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    bool res = va_get_expect_ids(lex, args);
    va_end(args);
    return res;
}

/// Arguments should be of type `Word` and end with WEND
bool get_expect_c(stb_lexer* lex, ...) {
    va_list args;
    va_start(args, lex);
    bool res = va_get_expect_c(lex, args);
    va_end(args);
    return res;
}

__BK_API void analyze_file(Schemas schemas, String content, CCompounds* out, bool derive_all) {
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
                if (peek_tokens(&lex, '}', -1)) break;
                Field field = {0};
                if (peek_c(&lex, ID("const"), ID("char"), TK('*'), TK(CLEX_id), WEND)) { 
                    stb_c_lexer_get_token(&lex); // const
                    stb_c_lexer_get_token(&lex); // char
                    stb_c_lexer_get_token(&lex); // *
                    field.type.kind = CPRIMITIVE;
                    field.type.type = CSTRING;
                } else if (peek_c(&lex, ID("char"), TK('*'), TK(CLEX_id), WEND)) {
                    stb_c_lexer_get_token(&lex); // char
                    stb_c_lexer_get_token(&lex); // *
                    field.type.kind = CPRIMITIVE;
                    field.type.type = CSTRING;
                } else if (peek_tokens(&lex, CLEX_id, CLEX_id, CLEX_id, -1)) {
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
                    } else {
                        // TODO: Report unknown primitive error
                        bk_log(LOG_INFO, "Unknown type while parsing struct, skipping...\n");
                        break;
                    }
                    
                } else if (peek_tokens(&lex, CLEX_id, CLEX_id, -1)) {
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
                        field.type.name = strdup(lex.string); // alloc
                    } else {
                        field.type.kind = CPRIMITIVE;
                    }
                } else {
                    // TODO: Report unknown field error
                    bk_log(LOG_INFO, "Unknown type while parsing struct, skipping...\n");
                    break;
                }
                stb_c_lexer_get_token(&lex); // name
                field.name = strdup(lex.string); // alloc
                if (!get_expect_tokens(&lex, ';', -1)) {
                    // TODO: get location info and report missing semicolon error
                    free_field(field);
                    break;
                }
                if (peek_c(&lex, ID("tag"), TK('('), TK(CLEX_dqstring), TK(')'), WEND)) {
                    stb_c_lexer_get_token(&lex); // tag
                    stb_c_lexer_get_token(&lex); // (
                    stb_c_lexer_get_token(&lex); // name
                    field.tag = strdup(lex.string); // alloc
                    stb_c_lexer_get_token(&lex); // )
                } else {
                    field.tag = NULL;
                }
                push_da(&strct.fields, field);
            }

            if (!get_expect_tokens(&lex, '}', CLEX_id, -1)) {
                free_ccompund(strct);
                continue;
            }
            strct.name = strdup(lex.string); // alloc
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

                stb_c_lexer_get_token(&lex); // derive_schema
                if (!matched) {
                    if (bk.conf.warn_unknown_attr) bk_log(LOG_WARN, "Found unknown attribute '%s' while parsing type '%s'\n", lex.string, strct.name);
                }
                stb_c_lexer_get_token(&lex); // (
                stb_c_lexer_get_token(&lex); // )
                
            }
            push_da(out, strct);
        } else {
            lex.parse_point = pp;
            stb_c_lexer_get_token(&lex);
        }
    }
}

size_t gen_dump_decl(Schemas schemas, String* book_buf, CCompound* ty, const char* dst_type) {
    size_t count = 0;
    if (ty->derived_schemas == 0) return count;
    gen_def_guard(BK_DUMP_UPPER);
    print_string(book_buf, "\n");
    for (size_t i = 0; i < schemas.len; ++i) {
        Schema* schema = schemas.items + i;
        if (ty->derived_schemas & (1 << i)) {
            if (schema->gen_dump_decl != NULL) {
                gen_def_type_guard(BK_DUMP_UPPER);
                count += schema->gen_dump_decl(book_buf, ty, dst_type);
                gen_endif_type_guard(BK_DUMP_UPPER);
            }
        }
    }
    gen_endif_guard(BK_DUMP_UPPER);
    return count;
}

size_t gen_parse_decl(Schemas schemas, String* book_buf, CCompound* ty) {
    size_t count = 0;
    if (ty->derived_schemas == 0) return count;
    gen_def_guard(BK_PARSE_UPPER);
    print_string(book_buf, "\n");
    for (size_t i = 0; i < schemas.len; ++i) {
        Schema* schema = schemas.items + i;
        if (ty->derived_schemas & (1 << i)) {
            if (schema->gen_parse_decl != NULL) {
                gen_def_type_guard(BK_PARSE_UPPER);
                count += schema->gen_parse_decl(book_buf, ty);
                gen_endif_type_guard(BK_PARSE_UPPER);
            }
        }
    }
    gen_endif_guard(BK_PARSE_UPPER);
    return count;
}

void gen_dump_impl(Schemas schemas, String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro) {
    if (ty->derived_schemas == 0) return;
    gen_def_guard(BK_DUMP_UPPER);
    print_string(book_buf, "\n");
    for (size_t i = 0; i < schemas.len; ++i) {
        Schema* schema = schemas.items + i;
        if (ty->derived_schemas & (1 << i)) {
            if (schema->gen_dump_impl != NULL) {
                gen_def_type_guard(BK_DUMP_UPPER);
                schema->gen_dump_impl(book_buf, ty, dst_type, fmt_macro);
                gen_endif_type_guard(BK_DUMP_UPPER);
            }
        }
    }
    gen_endif_guard(BK_DUMP_UPPER);
}

void gen_parse_impl(Schemas schemas, String* book_buf, CCompound* ty) {
    if (ty->derived_schemas == 0) return;
    gen_def_guard(BK_PARSE_UPPER);
    print_string(book_buf, "\n");
    for (size_t i = 0; i < schemas.len; ++i) {
        Schema* schema = schemas.items + i;
        if (ty->derived_schemas & (1 << i)) {
            if (schema->gen_parse_impl != NULL) {
                gen_def_type_guard(BK_PARSE_UPPER);
                schema->gen_parse_impl(book_buf, ty);
                gen_endif_type_guard(BK_PARSE_UPPER);
            }
        }
    }
    gen_endif_guard(BK_PARSE_UPPER);
}

// JSON generation
size_t gen_json_dump_decl(String* book_buf, CCompound* ty, const char* dst_type) {
    print_string(book_buf, "void dump_json_%s(%s* item, %s dst);\n", ty->name, ty->name, dst_type);
    return 1;
}
size_t gen_json_parse_decl(String* book_buf, CCompound* ty) {
    print_string(book_buf, "int parse_cjson_%s(cJSON* src, %s* dst);\n\n", ty->name, ty->name);
    print_string(book_buf, "/// WARN: Immediately returns on error, so `dst` might be partially filled.\n");
    print_string(book_buf, "int parse_json_%s(const char* src, unsigned long len, %s* dst);\n", ty->name, ty->name);
    return 2;
}
void gen_json_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro) {
    print_string(book_buf, "void dump_json_%s(%s* item, %s dst) {\n", ty->name, ty->name, dst_type);
    print_string(book_buf, "    BK_OFFSET_t offset = {0};\n");
    print_string(book_buf, "    (void)offset; // suppress warnings\n");
    print_string(book_buf, "    %s(\"{\");\n", fmt_macro);
    for (size_t j = 0; j < ty->fields.len; ++j) {
        Field* f = ty->fields.items + j;
        char* tag = f->tag ? f->tag : f->name;
        switch (f->type.kind) {
        case CPRIMITIVE: {
            switch (f->type.type) {
            case CINT: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%d\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CUINT: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%u\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CLONG: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%ld\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CULONG: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%lu\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CFLOAT: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%f\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CBOOL: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%s\", item->%s ? \"true\" : \"false\");\n", fmt_macro, tag, f->name);
            } break;
            case CSTRING: {
                // TODO: The generated code should escape item->field before printing it
                print_string(book_buf, "    %s(\"\\\"%s\\\":\\\"%%s\\\"\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CCHAR: {
                print_string(book_buf, "    %s(\"\\\"%s\\\":%%c\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            default: abort();
            }
        } break;
        case CEXTERNAL: {
            print_string(book_buf, "    %s(\"\\\"%s\\\":\");\n", fmt_macro, tag);
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
        char* tag = f->tag ? f->tag : f->name;
        print_string(book_buf, "    cJSON* %s_%s = cJSON_GetObjectItemCaseSensitive(src, \"%s\");\n", ty->name, f->name, tag);
        print_string(book_buf, "    if (!%s_%s) return 0;\n", ty->name, f->name);
        if (f->type.kind == CEXTERNAL) {
            print_string(book_buf, "    if (!parse_cjson_%s(%s_%s, &dst->%s)) return 0;\n", f->type.name, ty->name, f->name, f->name);
        } else if (f->type.kind == CPRIMITIVE) {
            switch (f->type.type) {
            case CINT: case CUINT: {
                print_string(book_buf, "    if (cJSON_IsNumber(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valueint;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
            } break;
            case CLONG: case CULONG: case CFLOAT: {
                print_string(book_buf, "    if (cJSON_IsNumber(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valuedouble;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
            } break;
            case CCHAR: {
                print_string(book_buf, "    if (cJSON_IsString(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        if (!%s_%s->valuestring) { return 0; };\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = *%s_%s->valuestring;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
                    
            } break;
            case CBOOL: {
                print_string(book_buf, "    if (cJSON_IsBool(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valueint;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return 0;\n");
                print_string(book_buf, "    }\n");
                    
            } break;
            case CSTRING: {
                print_string(book_buf, "    if (cJSON_IsString(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        if (!%s_%s->valuestring) { return 0; };\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = strdup(%s_%s->valuestring);\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
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
    print_string(book_buf, "/// WARN: Immediately returns on error, so `dst` might be partially filled.\n");
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
    print_string(book_buf, "    (void)offset; // suppress warnings\n");
    print_string(book_buf, "    %s(\"%s {\\n\");\n", fmt_macro, ty->name);
    for (size_t j = 0; j < ty->fields.len; ++j) {
        Field* f = ty->fields.items + j;
        char* tag = f->name;
        // char* tag = f->tag ? f->tag : f->name;
        switch (f->type.kind) {
        case CPRIMITIVE: {
            switch (f->type.type) {
            case CINT: {
                print_string(book_buf, "    %s(\"%%*s(int) %s: %%d\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CUINT: {
                print_string(book_buf, "    %s(\"%%*s(uint) %s: %%u\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CLONG: {
                print_string(book_buf, "    %s(\"%%*s(long) %s: %%ld\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CULONG: {
                print_string(book_buf, "    %s(\"%%*s(ulong) %s: %%lu\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CFLOAT: {
                print_string(book_buf, "    %s(\"%%*s(float) %s: %%f\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CBOOL: {
                print_string(book_buf, "    %s(\"%%*s(bool) %s: %%s\\n\", indent + 4, \"\", item->%s ? \"true\" : \"false\");\n", fmt_macro, tag, f->name);
            } break;
            case CSTRING: {
                // TODO: The generated code should escape item->field before printing it
                print_string(book_buf, "    %s(\"%%*s(string) %s: %%s\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            case CCHAR: {
                print_string(book_buf, "    %s(\"%%*s(char) %s: %%c\\n\", indent + 4, \"\", item->%s);\n", fmt_macro, tag, f->name);
            } break;
            default: abort();
            }
        } break;
        case CEXTERNAL: {
            print_string(book_buf, "    %s(\"%%*s%s: \", indent + 4, \"\");\n", fmt_macro, tag);
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

#ifdef BK_ENABLE_BK_CONF_GEN
// bkconf generation
size_t gen_bkconf_parse_decl(String* book_buf, CCompound* ty) {
    print_string(book_buf, "int parse_bkconf_%s(const char* src, unsigned long len, %s* dst);\n", ty->name, ty->name);
    return 1;
}
void gen_bkconf_parse_impl(String* book_buf, CCompound* ty) {
    print_string(book_buf, "int parse_bkconf_%s(const char* src, unsigned long len, %s* dst) {\n", ty->name, ty->name);
    // print_string(book_buf, "    int powi(int a, int b) { int res = 1; for (int i = 0; i < b; ++i) { res *= a; } return res; }\n");
    print_string(book_buf, "    char str_buf[512] = {0};\n");
    print_string(book_buf, "    const char* name_start = src;\n");
    print_string(book_buf, "    const char* name_end = src;\n");
    print_string(book_buf, "    const char* value_start = src;\n");
    print_string(book_buf, "    const char* value_end = src;\n");
    print_string(book_buf, "    double value_double = 0; (void)value_double;\n");
    print_string(book_buf, "    long value_int = 0;\n");
    print_string(book_buf, "    bool value_bool = false;\n");
    print_string(book_buf, "    for (const char* cur = src; cur < (src + len); ++cur) {\n");
    print_string(book_buf, "        if (*cur == '#') {\n");
    print_string(book_buf, "            for (; cur < (src + len); ++cur) {\n");
    print_string(book_buf, "                if (*cur == '\\n') {\n");
    print_string(book_buf, "                    ++cur;\n");
    print_string(book_buf, "                    name_start = cur;\n");
    print_string(book_buf, "                    break;\n");
    print_string(book_buf, "                }\n");
    print_string(book_buf, "            }\n");
    print_string(book_buf, "        } else if (*cur == '\\n') {\n");
    print_string(book_buf, "            value_end = cur - 1;\n");
    print_string(book_buf, "            value_int = atol(value_start);\n");
    print_string(book_buf, "            value_double = atof(value_start);\n");
    print_string(book_buf, "            if (strncmp(value_start, \"true\", 4) == 0) value_bool = true;\n");
    print_string(book_buf, "            if (strncmp(value_start, \"false\", 5) == 0) value_bool = false;\n");
    print_string(book_buf, "            str_buf[sprintf(str_buf, \"%%.*s\", (int)(name_end - name_start) + 1, name_start)] = 0;\n");
    for (size_t i = 0; i < ty->fields.len; ++i) {
        if (i == 0) {
            print_string(book_buf, "            if (strcmp(str_buf, \"%s\") == 0) {\n", ty->fields.items[i].name);
        } else {
            print_string(book_buf, " else if (strcmp(str_buf, \"%s\") == 0) {\n", ty->fields.items[i].name);
        }
        switch (ty->fields.items[i].type.kind) {

        case CPRIMITIVE: {
            switch (ty->fields.items[i].type.type) {

            case CINT: case CUINT: case CLONG: case CULONG: {
                print_string(book_buf, "                dst->%s = value_int;\n", ty->fields.items[i].name);
            } break;
            case CCHAR: {
                print_string(book_buf, "                dst->%s = *value_start;\n", ty->fields.items[i].name);
            } break;
            case CFLOAT: {
                print_string(book_buf, "                dst->%s = value_double;\n", ty->fields.items[i].name);
            } break;
            case CBOOL: {
                print_string(book_buf, "                dst->%s = value_bool;\n", ty->fields.items[i].name);
            } break;
            case CSTRING: {
                print_string(book_buf, "                str_buf[sprintf(str_buf, \"%%.*s\", (int)(value_end - value_start) + 1, value_start)] = 0;\n");
                print_string(book_buf, "                dst->%s = strdup(str_buf);\n", ty->fields.items[i].name);
                print_string(book_buf, "                str_buf[sprintf(str_buf, \"%%.*s\", (int)(name_end - name_start) + 1, name_start)] = 0;\n");
            } break;
            }
        } break;
        case CEXTERNAL: {
            bk_log(LOG_ERROR, "CEXTERNAL NOT SUPPORTED IN BKCONF\n");
            abort();         
        } break;
        }

        print_string(book_buf, "            }");
    }
    print_string(book_buf, "\n");

    print_string(book_buf, "            name_start = cur + 1;\n");
    print_string(book_buf, "        } else if (*cur == '=') {\n");
    print_string(book_buf, "            name_end = cur - 1;\n");
    print_string(book_buf, "            value_start = cur + 1;\n");
    print_string(book_buf, "        }\n");
    print_string(book_buf, "    }\n");
    print_string(book_buf, "    return 1;\n");
    print_string(book_buf, "}\n");
}
#endif // BK_ENABLE_BK_CONF_GEN

bool help_cmd(int* i, int argc, char** argv) {
    (void)bk;

    if ((*i + 1) < argc) {
        char* arg = argv[*i + 1];
        if (strlen(arg) < 1) return false;
        if (arg[0] != '-') {
            bool found = false;
            for (size_t j = 0; j < commands_count; ++j) {
                if (strcmp(arg, commands[j].name) == 0) {
                    found = true;
                    bk_printf("%s:\n    Usage: %s\n    Description: %s\n\n", commands[j].name, commands[j].usage, commands[j].desc);
                    break;
                }
            }
            return found;
        }
    }
    bk_printf("[help start]\n\n");
    for (size_t j = 0; j < commands_count; ++j) {
        bk_printf("%s: %s\n\n", commands[j].name, commands[j].desc);
    }
    bk_printf("[help end]\n\n");

    return true;    
}


bool config_path_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        config_path = argv[*i];
        return true;
    }
    return false;
}

bool output_mode_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.output_mode = argv[*i];
        return true;
    }

    return false;    
}

bool gen_ext_cmd(int* i, int argc, char** argv) {
    char* src = NULL;
    char* out = NULL;

    if (++*i < argc) {
        src = argv[*i];
    }

    if (++*i < argc) {
        out = argv[*i];
    }

    if (src == NULL || out == NULL) return false;
    
    const char* def_str = "#ifndef __BK_GEN_EXT_DEFINITIONS";
    size_t def_strlen = strlen(def_str);
    const char* endif_str = "#endif // __BK_GEN_EXT_DEFINITIONS";
    size_t endif_strlen = strlen(def_str);

    char* save_start = NULL;
    char* save_end = NULL;

    String src_file = {0}; // alloc
    if (!read_entire_file(src, &src_file) || src_file.len == 0) {
        if (src_file.items != NULL) free(src_file.items);
        return false;
    }

    for (size_t i = 0; i < src_file.len; ++i) {
        char* p = src_file.items + i;
        size_t p_len = strlen(p);
        if (save_start == NULL && p_len >= def_strlen && strncmp(def_str, p, def_strlen) == 0) {
            save_start = p;
        }
        if (save_end == NULL && save_start != NULL && p_len >= endif_strlen && strncmp(endif_str, p, endif_strlen) == 0) {
            save_end = p + endif_strlen;
        }
    }

    if (save_start == NULL || save_end == NULL) {
        free(src_file.items);
        return false;
    }

    FILE* out_file = fopen(out, "wb");
    if (!out_file) {
        bk_log(LOG_ERROR, "Couldn't open file '%s': '%s'\n", out, strerror(errno));
        free(src_file.items);
        return false;
    }
    fprintf(out_file,
        "/*\n"
        "Copyright (c) 2025 Serdar Çoruhlu <serdar.coruhlu@hotmail.com>\n"
        "\n"
        "Permission is hereby granted, free of charge, to any\n"
        "person obtaining a copy of this software and associated\n"
        "documentation files (the \"Software\"), to deal in the\n"
        "Software without restriction, including without\n"
        "limitation the rights to use, copy, modify, merge,\n"
        "publish, distribute, sublicense, and/or sell copies of\n"
        "the Software, and to permit persons to whom the Software\n"
        "is furnished to do so, subject to the following\n"
        "conditions:\n"
        "\n"
        "The above copyright notice and this permission notice\n"
        "shall be included in all copies or substantial portions\n"
        "of the Software.\n"
        "\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF\n"
        "ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED\n"
        "TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A\n"
        "PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT\n"
        "SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n"
        "CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION\n"
        "OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR\n"
        "IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
        "DEALINGS IN THE SOFTWARE.\n"
        "*/\n"
    );
    fwrite(save_start, save_end - save_start, sizeof *save_start, out_file);

    fclose(out_file);
    
    free(src_file.items);
    return true;
}

bool generics_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk.conf.generics = true;
    return true;
}

bool watch_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk.conf.watch_mode = true;
    return true;
}

bool watch_delay_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        char* endptr = NULL;
        long val = strtol(argv[*i], &endptr, 10);
        if (endptr && *endptr == 0) {
            bk.conf.watch_delay = val;
            return true;
        }
        return false;
    }
    return false;
}

bool include_file_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        char* ent = argv[*i];
        Entry e = {0};
        if (!entry_from_file(ent, &e)) return false; // alloc
        push_da(&bk.entries, e);
        return true;
    }
    return false;
}

bool include_directory_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.include_dir = argv[*i];
        return true;
    }
    return false;
}

bool output_directory_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.output_dir = argv[*i];
        return true;
    }
    return false;
}

bool schemas_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    if (bk.schemas.len == 0) {
        bk_printf("No schemas were loaded.\n");
        return true;
    } else {
        bk_printf("Loaded schemas: ");
    }
    for (size_t i = 0; i < bk.schemas.len; ++i) {
        if (i == 0) {
            bk_printf("%s", bk.schemas.items[i].name);
        } else {
            bk_printf(", %s", bk.schemas.items[i].name);
        }
    }
    bk_printf("\n");
    return true;
}

bool silent_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk.conf.silent = true;
    return true;
}

bool verbose_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk.conf.verbose = true;
    return true;
}

bool enable_warn_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        char* w = argv[*i];
        if (strcmp(w, WARN_NO_INCLUDE) == 0) {
            bk.conf.warn_no_include = true;
            return true;
        } else if (strcmp(w, WARN_NO_OUTPUT) == 0) {
            bk.conf.warn_no_output = true;
            return true;
        } else if (strcmp(w, WARN_UNKNOWN_ATTR) == 0) {
            bk.conf.warn_unknown_attr = true;
            return true;
        } else {
            return false;
        }
    }

    return false;
}

bool disable_warn_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        char* w = argv[*i];
        if (strcmp(w, WARN_NO_INCLUDE) == 0) {
            bk.conf.warn_no_include = false;
            return true;
        } else if (strcmp(w, WARN_NO_OUTPUT) == 0) {
            bk.conf.warn_no_output = false;
            return true;
        } else if (strcmp(w, WARN_UNKNOWN_ATTR) == 0) {
            bk.conf.warn_unknown_attr = false;
            return true;
        } else {
            return false;
        }
    }

    return false;
}

bool derive_all_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk.conf.derive_all = true;
    return true;
}

bool gen_impl_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.gen_implementation_macro = argv[*i];
        return true;
    }
    return false;
}

bool gen_fmt_dst_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.gen_fmt_dst_macro = argv[*i];
        return true;
    }
    return false;
}

bool gen_fmt_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.gen_fmt_macro = argv[*i];
        return true;
    }
    return false;
}

bool offset_type_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.offset_type_macro = argv[*i];
        return true;
    }
    return false;
}

bool disable_prefix_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.disable_macro_prefix = argv[*i];
        return true;
    }
    return false;
}

bool enable_prefix_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        bk.conf.enable_macro_prefix = argv[*i];
        return true;
    }
    return false;
}

bool disable_dump_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk.conf.disable_dump = true;
    return true;
}

bool disable_parse_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk.conf.disable_parse = true;
    return true;
}

bool disabled_cmd(int* i, int argc, char** argv) {
    (void)i;
    (void)argc;
    (void)argv;
    bk.conf.disabled_by_default = true;
    return true;
}

static char* parse_list(char** src, char sep, size_t* entry_len) {
    if (src == NULL || *src == NULL || **src == 0) return NULL;

    if (**src == sep) return NULL;

    char* begin = *src;
    size_t len = strlen(begin);
    char* end = begin + len;
    for (;*src < end; (*src)++) {
        if (**src == ',') {
            *entry_len = (*src)++ - begin;
            return begin;
        }
    }
    if (len > 0) {
        *entry_len = len;
        return begin;
    }
    return NULL;
}

// Generated code, generated by enabling `BK_ENABLE_BK_CONF_GEN`
int parse_bkconf_BkConfig(const char* src, unsigned long len, BkConfig* dst) {
    char str_buf[512] = {0};
    const char* name_start = src;
    const char* name_end = src;
    const char* value_start = src;
    const char* value_end = src;
    double value_double = 0; (void)value_double;
    long value_int = 0;
    bool value_bool = false;
    for (const char* cur = src; cur < (src + len); ++cur) {
        if (*cur == '#') {
            for (; cur < (src + len); ++cur) {
                if (*cur == '\n') {
                    ++cur;
                    name_start = cur;
                    break;
                }
            }
        } else if (*cur == '\n') {
            value_end = cur - 1;
            value_int = atol(value_start);
            value_double = atof(value_start);
            if (strncmp(value_start, "true", 4) == 0) value_bool = true;
            if (strncmp(value_start, "false", 5) == 0) value_bool = false;
            str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            if (strcmp(str_buf, "output_mode") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->output_mode = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "generics") == 0) {
                dst->generics = value_bool;
            } else if (strcmp(str_buf, "silent") == 0) {
                dst->silent = value_bool;
            } else if (strcmp(str_buf, "verbose") == 0) {
                dst->verbose = value_bool;
            } else if (strcmp(str_buf, "warn_unknown_attr") == 0) {
                dst->warn_unknown_attr = value_bool;
            } else if (strcmp(str_buf, "warn_no_include") == 0) {
                dst->warn_no_include = value_bool;
            } else if (strcmp(str_buf, "warn_no_output") == 0) {
                dst->warn_no_output = value_bool;
            } else if (strcmp(str_buf, "disable_dump") == 0) {
                dst->disable_dump = value_bool;
            } else if (strcmp(str_buf, "disable_parse") == 0) {
                dst->disable_parse = value_bool;
            } else if (strcmp(str_buf, "disabled_by_default") == 0) {
                dst->disabled_by_default = value_bool;
            } else if (strcmp(str_buf, "watch_mode") == 0) {
                dst->watch_mode = value_bool;
            } else if (strcmp(str_buf, "watch_delay") == 0) {
                dst->watch_delay = value_int;
            } else if (strcmp(str_buf, "gen_fmt_macro") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->gen_fmt_macro = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "gen_implementation_macro") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->gen_implementation_macro = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "gen_fmt_dst_macro") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->gen_fmt_dst_macro = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "offset_type_macro") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->offset_type_macro = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "disable_macro_prefix") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->disable_macro_prefix = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "enable_macro_prefix") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->enable_macro_prefix = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "derive_all") == 0) {
                dst->derive_all = value_bool;
            } else if (strcmp(str_buf, "include_dir") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->include_dir = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "include_files") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->include_files = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            } else if (strcmp(str_buf, "output_dir") == 0) {
                str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
                dst->output_dir = strdup(str_buf);
                str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;
            }
            name_start = cur + 1;
        } else if (*cur == '=') {
            name_end = cur - 1;
            value_start = cur + 1;
        }
    }
    return 1;
}

#endif // __BOOKKEEPER_GEN_C__
