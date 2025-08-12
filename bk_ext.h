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
#ifndef __BK_GEN_EXT_DEFINITIONS
#define __BK_GEN_EXT_DEFINITIONS

#include <stddef.h>
#include <time.h>
#include <stdbool.h>

#define derive_bkconf(...)

typedef struct {
    char* output_mode;
    bool silent;
    bool verbose;
    bool warn_unknown_attr;
    bool warn_no_include;
    bool warn_no_output;
    bool disable_dump;
    bool disable_parse;
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
