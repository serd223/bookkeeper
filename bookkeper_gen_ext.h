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

static char tmp_str[4096];
#define tfmt(...) (sprintf(tmp_str, __VA_ARGS__), tmp_str)
#define fmt(...) strdup(tfmt(__VA_ARGS__))

#define bk_log_loc(level, source, line, ...) do {\
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
#endif // __BK_GEN_EXT_DEFINITIONS
