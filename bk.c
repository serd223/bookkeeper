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

/** @file bk.c
    The main source file of bookkeeper.
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

#ifndef __BK_GEN_EXT_DEFINITIONS
#define __BK_GEN_EXT_DEFINITIONS

// This would typically be in drives.h, but the single file limitation strikes again
/** @cond */
#define derive_bkconf(...)
/** @endcond */

/**
 * @brief The general config of `bookkeeper`. Can be configured via command line arguments or configuration files.
*/
typedef struct {
    /**
     * @brief Is expected to be either "mirror" or "dir". ("mirror" by default)
     *
     * The program will exit with a nonzero exit code if this string is something else.
     *
     * "mirror" mode puts files that were generated from the user source code
     * next to its source file, "mirror"ing the original file structure.
     *
     * "dir" mode puts all generated files inside the specified `BkConfig::output_dir`.
    */
    char* output_mode;

    /**
     * @brief Enables the experimental generated generic macro API. (false by default)
     *
     * This API is still very experimental and isn't stabilized.
    */
    bool generics;

    /** @brief Disables all program output. (true by default) */
    bool silent;

    /**
     * @brief Enables verbose output. (false by default)
     *
     * When disabled, @link LogLevel `LogLevel::LOG_ERROR`@endlink and
     * @link LogLevel `LogLevel::LOG_WARN`@endlink level logs will be displayed without
     * the file location.
     *
     * When enabled, logs of all levels are displayed and they all contain file
     * location information.
    */
    bool verbose;

    /**
     * @brief Enables the warning that is emitted when an unknown attribute is found inside
     *        analyzed source code. (true by default)
    */
    bool warn_unknown_attr;

    /** @brief Enables the warning that is emitted when no files were included (false by default) */
    bool warn_no_include;

    /** @brief Enables the warning that is emitted when no `output_directory` was specified (true by default) */
    bool warn_no_output;

    /** @brief Disables the generation of "dump" functions during code generation (false by default) */
    bool disable_dump;

    /** @brief Disables the generation of "parse" functions during code generation (false by default) */
    bool disable_parse;

    /**
     * @brief Enables the experimental "disabled by default" API for generated code. (false by default)
     *
     * When enabled, all features in the generated code are disabled and users
     * are expected to pick and choose the features they want to enable
     * and enable them with BK_ENABLE_* macros.
     *
     * Since this mode introduces a lot of additional complexity for users,
     * it may also be called "advanced mode".
     *
     * This mode might be temporarily removed for the first release of `bookkeeper`.
     *
     * This API is still very experimental and isn't stabilized.
    */
    bool disabled_by_default;

    /** @brief Enables watch mode that "watches" your source files and auto-generates code automatically (false by default) */
    bool watch_mode;

    /**
     * Simple hack to prevent watch mode from hogging resources. The tool waits `BkConfig::watch_delay` seconds before checking
     * if any source files were modified to regenerate code (5 by default)
    */
    long watch_delay;

    /**
     * @brief The macro that is used inside generated "dump" functions to output into the provided "dst" buffer.
     *        ("BK_FMT" by default)
    */
    char* gen_fmt_macro;

    /**
     * @brief The macro that is used inside generated code to include implementation in stb-style.
     *        ("BK_IMPLEMENTATION" by default)
    */
    char* gen_implementation_macro;

    /**
     * @brief The macro that is used inside generated "dump" functions to denote the type of the "dst" parameter.
     *        ("BK_FMT_DST_t" by default)
    */
    char* gen_fmt_dst_macro;

    /**
     * @brief The macro that is used inside generated "dump" functions to denote the type of the "offset" local
     *        variable that is used inside the function. ("BK_OFFSET_t" by default)
    */
    char* offset_type_macro;

    /**
     * @brief The prefix that is used to generate macros that disable specific parts of generated code
     *        ("BK_DISABLE_" by default)
     *
     * These generated macros can be:
     *  - $prefix$DUMP: Disables dump functionality.
     *  - $prefix$PARSE: Disables parse functionality.
     *  - $prefix$$type$: Disables all functionality that belongs to $type$.
     *  - $prefix$$type$_DUMP: Disables dump functionality that belongs to $type$.
     *  - $prefix$$type$_PARSE: Disables parse functionality that belongs to $type$.
     *  - $prefix$$type$_$schema$: Disables all functionality that works on $schema$ and belongs to $type$.
     *  - $prefix$$type$_$schema$_DUMP: Disables dump functionality that works on $schema$ and belongs to $type$.
     *  - $prefix$$type$_$schema$_PARSE: Disables parse functionality that works on $schema$ and belongs to $type$.
     *  - $prefix$$schema$: Disables all functionality that works on $schema$.
     *  - $prefix$$schema$_DUMP: Disables dump functionality that works on $schema$.
     *  - $prefix$$schema$_PARSE: Disables parse functionality that works on $schema$.
    */
    char* disable_macro_prefix;

    /**
     * @brief The prefix that is used to generate enable macros for the "disabled by default" API.
     *        ("BK_ENABLE_" by default)
     *
     * This API is still very experimental and isn't stabilized.
    */
    char* enable_macro_prefix;

    /**
     * @brief Derives all possible schemas for every struct that is analyzed. (false by default)
     *
     * Mainly used for debugging issues regarding generated code.
    */
    bool derive_all;

    /**
     * @brief Denotes the directory that will be searched for '.c' or '.h' files to analyze.
     *
     * Set to NULL if no include directory was specified.
    */
    char* include_dir;

    /**
     * @brief Only used when reading input files as a comma seperated list from configuration files.
     *
     * Set to NULL if no config file was loaded, or this field wasn't set inside the loaded config file.
    */
    char* include_files;

    /**
     * @brief Only used when reading dynamic schema files as a comma seperated list from configuration files.
     *
     * Set to NULL if no config file was loaded, or this field wasn't set inside the loaded config file.
    */
    char* schema_files;

    /**
     * @brief Denotes the directory that will be used to output most(*) generated files.
     *
     * Only used for 'derives.h' and 'generics.h' when `BkConfig::output_mode` is "mirror".
     *
     * (*): The 'out' file specified inside the '--gen-ext' flag isn't placed inside this directory
     *      and that path is directly passed to `fopen`.
    */
    char* output_dir;
} BkConfig derive_bkconf();

/**
 * @brief Sized dynamic string. Just a dynamic array of characters.
 *
 * Its memory is dynamically allocated by macros like @link push_da `push_da`@endlink
 * and @link print_string `print_string`@endlink.
 *
 * You are expected to `free` that memory yourself.
*/
typedef struct {
    char* items;
    size_t len;
    size_t cap;
} String;

/** @brief Dynamic array of `String`. */
typedef struct {
    String* items;
    size_t len;
    size_t cap;
} Strings;

/** @cond */

// String_View stuff
typedef struct {
    const char* items;
    size_t len;
} String_View;

// NOTE: Perhaps we could also generate macros like this for dumping?
#define SV_FMT "%.*s"
#define SV_ARG(sv) (int)(sv).len, (sv).items
#define SV_ARG_N(sv, n) (int)((n <= (sv).len) ? n : (sv).len), (sv).items

String_View sv(const char* cstr);
String_View sv_from_parts(const char* items, size_t len);
String_View sv_from_str(String* str);
char* sv_to_cstr(String_View sv);
bool sv_starts_with(String_View sv, String_View prefix);
String_View sv_chop(String_View sv, size_t n);
bool sv_chop_if_prefix(String_View* sv, String_View prefix);
String_View sv_chop_line(String_View sv, size_t n);
void sv_chop2(String_View* sv, size_t n);
String_View sv_trim_start(String_View sv, char c);
String_View sv_trim2_start(String_View sv, String_View cs);
String_View sv_trim_whitespace_start(String_View sv);
String_View sv_trim_whitespace_end(String_View sv);
String_View sv_trim_whitespace(String_View sv);
String_View sv_find(String_View sv, char c);
String_View sv_substr(String_View sv, size_t start, size_t len);
void sv_loc(String_View file, String_View cursor, int* line, int* offset);

/** @endcond */

/** @brief Kind tag for `CType` */
typedef enum {
    CPRIMITIVE,
    CEXTERNAL
} CType_Kind;

/** @brief Defines different primitive C types. */
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

/**
 * @brief Defines a C type. Acts like a tagged union with the `CType::kind` tag.
 *
 * This struct isn't a tagged union to keep its size predictable (Mainly for use in dynamic arrays).
*/
typedef struct {
    /** @brief Tag that denotes which variant the current instance of this type belongs to. */
    CType_Kind kind;

    /**
     * @brief Denotes the name of the external type.
     *
     * %Field of variant @link CType_Kind `CType_Kind::CEXTERNAL`. @endlink
    */
    char* name;

    /**
     * @brief Used to identify the primitive type of this type.
     *
     * %Field of variant @link CType_Kind `CType_Kind::CPRIMITIVE`. @endlink
    */
    CPrimitive type;
} CType;

/** @brief Dynamic array of `CType` */
typedef struct {
    CType* items;
    size_t len;
    size_t cap;
} CTypes;

/**
 * @brief Defines a field stored inside `CCompound` types.
*/
typedef struct {
    /** @brief The identifier that was used to declare this field inside the analyzed source file. */
    char* name;

    /**
     * @brief Optionally declared inside the source file with the 'tag' attribute.
     *
     * If it was declared inside the source file, it is set to the string specified inside the attribute invocation. If
     * a special tag was not declared inside the source file, `Field::tag` is set to NULL and code generation uses the `Field::name`
     * field for serialization.
    */
    char* tag;

    /** @brief The @link CType type@endlink of this field. */
    CType type;
} Field;

/** @brief Dynamic array of `Field` */
typedef struct {
    Field* items;
    size_t len;
    size_t cap;
} Fields;

/**
 * @brief Defines a compund C type with fields.
*/
typedef struct {
    int derived_schemas; // TODO: this bitfield method puts a (somewhat low) hard limit on the amount of allowed schmeas
    Fields fields;
    char* name;
} CCompound;

/** @brief Dynamic array of `CCompound` */
typedef struct {
    CCompound* items;
    size_t len;
    size_t cap;
} CCompounds;

/** @brief Defines a static schema. */
typedef struct {
    /**
     * @brief (Nullable) Pointer to function that will be called to generate 'prelude' code that will be generated once at the
     *                   top of each generated file.
     *
     * Schema authors may use this section for generating common type definitions (such as return enumerations) or for
     * utility functions to be used in generated code.
     *
     * The code inside generated inside function is automatically protected by extra header guards
     * to avoid redefinition/redeclaration errors.
     *
     * @param book_buf The string buffer that is used to store generated code. The @link print_string `print_string` @endlink
    */
    void (*gen_prelude)(String* book_buf);

    /**
     * @brief (Nullable) Pointer to function that will be used to generate the declarations of "dump"
     *        functions for the specified type.
     *
     * The convention for the signatures of generated functions here is `void dump_$schema.name$_$type$($type$* item, $dst_type$ dst)`
     *
     * @param book_buf The string buffer that is used to store generated code. The @link print_string `print_string` @endlink
     *        macro available to both this source file and extension authors can be used to output into this buffer.
     * @param ty The type that this function is meant to generate code for, contains all necessary info for code generation.
     * @param dst_type The type of the 'dst' parameter that is meant to be used for generated "dump" functions.
     * @return Should return the total amount of declarations.
    */
    size_t (*gen_dump_decl)(String* book_buf, CCompound* ty, const char* dst_type);

    /**
     * @brief (Nullable) Pointer to function that will be used to generate the declarations of "parse"
     *        functions for the specified type.
     *
     * The convention for the signatures of generated functions here is `int|$enum$ parse_$schema$_$type$(char* src, unsigned long len, $type$* dst)`
     *
     * Schemas may also define enumerations inside their `StaticSchema::gen_prelude` functions to return error codes. Even if schema
     * authors make use of such error codes, return value '0' should always mean OK.
     *
     *
     * @param book_buf The string buffer that is used to store generated code. The @link print_string `print_string` @endlink
     *        macro available to both this source file and extension authors can be used to output into this buffer.
     * @param ty The type that this function is meant to generate code for, contains all necessary info for code generation.
     * @return Should return the total amount of declarations.
    */
    size_t (*gen_parse_decl)(String* book_buf, CCompound* ty);

    /**
     * @brief (Nullable) Pointer to function that will be used to generate the implementations of "dump"
     *        functions for the specified type.
     *
     * The convention for the signatures of generated functions here is `void dump_$schema.name$_$type$($type$* item, $dst_type$ dst)`
     *
     * @param book_buf The string buffer that is used to store generated code. The @link print_string `print_string` @endlink
     *        macro available to both this source file and extension authors can be used to output into this buffer.
     * @param ty The type that this function is meant to generate code for, contains all necessary info for code generation.
     * @param dst_type The type of the 'dst' parameter that is meant to be used for generated "dump" functions.
     * @param fmt_macro The name of the macro that is meant to be used **inside generated code** to output into the 'dst' buffer.
    */
    void (*gen_dump_impl)(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);

    /**
     * @brief (Nullable) Pointer to function that will be used to generate the implementations of "parse"
     *        functions for the specified type.
     *
     * The convention for the signatures of generated functions here is `int|$enum$ parse_$schema$_$type$(char* src, unsigned long len, $type$* dst)`
     *
     * Schemas may also define enumerations inside their `StaticSchema::gen_prelude` functions to return error codes. Even if schema
     * authors make use of such error codes, return value '0' should always mean OK.
     *
     * @param book_buf The string buffer that is used to store generated code. The @link print_string `print_string` @endlink
     *        macro available to both this source file and extension authors can be used to output into this buffer.
     * @param ty The type that this function is meant to generate code for, contains all necessary info for code generation.
    */
    void (*gen_parse_impl)(String* book_buf, CCompound* ty);

    /**
     * @brief Defines the name of the 'attribute macro' that will be generated inside 'derives.h'
     *        for users to derive functionality for this schema.
    */
    const char* derive_attr;

    /**
     * @brief Defines the unique name for this schema.
    */
    const char* name;
} StaticSchema;

/** @brief Dynamic array for `StaticSchema`. */
typedef struct {
    StaticSchema* items;
    size_t len;
    size_t cap;
} StaticSchemas;

/** @brief Defines a dynamic schema object. */
typedef struct {    
    /**
     * @brief String view that contains a view into the source code of the dynamic schema in the .schema format.
     *
     * The memory that is pointed to by this view is managed by the global `BkState` instance.
    */
    String_View source;

    /**
     * @brief Defines the name of the 'attribute macro' that will be generated inside 'derives.h'
     *        for users to derive functionality for this dynamic schema.
     *
     * The memory that is pointed to by this view is managed by the global `BkState` instance.
    */
    String_View derive_attr;

    /**
     * @brief Defines the unique name for this dynamic schema.
     *
     * The memory that is pointed to by this view is managed by the global `BkState` instance.
    */
    String_View name;
} DynamicSchema;

/** @brief Dynamic array for `DynamicSchema`. */
typedef struct {
    DynamicSchema* items;
    size_t len;
    size_t cap;
} DynamicSchemas;

/** @brief Enumeration that defines all supported schema types. */
typedef enum {
    SCHEMA_STATIC,
    SCHEMA_DYNAMIC,
} SchemaType;

/**
 * @brief Defines a source file meant to be analyzed.
 *
 * The contents of `Entry::full` and `Entry::name` aren't validated in any way and are expected to be
 * assigned correctly.
*/
typedef struct {
    /**
     * @brief The canonical path to the entry file.
     *
     * Used during logging and generating unique hashes for generated files.
    */
    char* full;
    /**
     * @brief The short display name for the entry file.
     *
     * Used for logging and human readable generated code.
     * Ideally would be the last part of a path string.
    */
    char* name;
    /**
     * @brief The modification time (in seconds) of the entry file in the file system.
     *
     * When using `stat` on POSIX, this field is equivalent to `file_stat.st_mtim.tv_sec`.
    */
    time_t sys_modif;
    /**
     * Assigned to the time elapsed since the UNIX epoch (in seconds)
     * whenever this file is analyzed (even if the analysis fails).
    */
    time_t last_analyzed;
} Entry;

/** @brief Dynamic array for `Entry`. */
typedef struct {
    Entry* items;
    size_t len;
    size_t cap;
} Entries;

/** @brief General state of `bookkeeper`. */
typedef struct {
    /** @brief The current configuration for `bookkeeper`. */
    BkConfig conf;

    /** @brief Dynamic array of all input files that were discovered or supplied. */
    Entries entries;

    /** @brief Dynamic array of all static schemas that were defined. */
    StaticSchemas schemas;

    /** @brief Dynamic array of all static schemas that were defined. */
    DynamicSchemas dynamic_schemas;

    /** @brief Dynamic string buffer that is used to store all dynamic schema sources. */
    Strings dynamic_source;

    /** @brief Bitfield that contains all schemas that were derived globally */
    int derive_schemas;
} BkState;

/**
 * @var bk
 * Global static `BkState` instance. (not thread safe)
*/
static BkState bk = {0};
/**
 * @var tmp_str
 * Global static temporary string buffer. (not thread safe)
*/
static char tmp_str[4096];

/**
 * @brief Uses the global @link tmp_str `tmp_str`@endlink buffer with `sprintf` to quickly format a string.
 *
 * The `tmp_str` gets constantly overwritten with every call to this macro or other functions that use the buffer
 * so this macro shouldn't be used to generate strings that are meant to be stored.
 *
 * @return Returns the pointer to the global `tmp_str` buffer so this macro can easily be used inside expressions.
*/
#define tfmt(...) (sprintf(tmp_str, __VA_ARGS__), tmp_str)

/**
 * @brief Internally uses the @link tfmt `tfmt`@endlink macro to format a string, but this macro also duplicates that string for safe storage.
 *
 * Has to be manually free'd with `free`.
*/
#define fmt(...) strdup(tfmt(__VA_ARGS__))

/**
 * @brief Logging system of `bookkeeper`.
 *
 * @param level Level of logging meant to be used for this call. Should be a member of `LogLevel`.
 * @param source The path to the source file name that will be used while logging.
 * @param line The line number that will be used while logging.
 * @param ... `printf` style format arguments
*/
#define bk_log_loc(level, source, line, ...) do {\
    if (!bk.conf.silent) {\
        switch ((level)) {\
        case LOG_INFO: {\
            if (bk.conf.verbose) {\
                fprintf(stderr, "%s:%d: [INFO] ", (source), (line));\
                fprintf(stderr, __VA_ARGS__);\
            }\
        } break;\
        case LOG_WARN: {\
            if (bk.conf.verbose) {\
                fprintf(stderr, "%s:%d: [WARN] ", (source), (line));\
            } else {\
                fprintf(stderr, "WARNING: ");\
            }\
            fprintf(stderr, __VA_ARGS__);\
        } break;\
        case LOG_ERROR: {\
            if (bk.conf.verbose) {\
                fprintf(stderr, "%s:%d: [ERROR] ", (source), (line));\
            } else {\
                fprintf(stderr, "ERROR: ");\
            }\
            fprintf(stderr, __VA_ARGS__);\
        } break;\
        default: abort();\
        }\
    }\
} while(0)

/**
 * @brief Wrapper around @link bk_log_loc `bk_log_loc`,@endlink used for internal logging.
 *
 * @param level Level of logging meant to be used for this call. Should be a member of `LogLevel`.
 * @param ... `printf` style format arguments
*/
#define bk_log(level, ...) bk_log_loc(level, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Diagnostics system of `bookkeeper`.
 *
 * This macro is purely for use in @link analyze_file `analyze_file`@endlink since it implicitly uses
 * the `file_name` and `lex` parameters of the function.
 *
 * See @link bk_diag_loc `bk_diag_loc`@endlink for a generic version that accepts those as parameters.
 *
 * @param level Level of logging meant to be used for this call. Should be a member of `LogLevel`.
 * @param ... `printf` style format arguments
*/
#define bk_diag(level, ...) do {\
stb_lex_location loc = {0};\
stb_c_lexer_get_location(&lex, lex.where_firstchar, &loc);\
bk_diag_loc(level, file_name, loc.line_number, loc.line_offset, __VA_ARGS__);\
} while(0)

/**
 * @brief Inner `loc` implementation of @link bk_diag `bk_diag`@endlink.
 *
 * @param level Level of logging meant to be used for this call. Should be a member of `LogLevel`.
 * @param source Short name of the source file that the diagnostic originated from.
 * @param line The line number the diagnostic originated at.
 * @param offset The line offset the diagnostic originated at.
 * @param ... `printf` style format arguments
*/
#define bk_diag_loc(level, source, line, offset, ...) do {\
    switch ((level)) {\
    case LOG_INFO: {\
        if (bk.conf.verbose) {\
            fprintf(stderr, "%s:%d:%d: ", (source), (line), (offset));\
            fprintf(stderr, __VA_ARGS__);\
        }\
    } break;\
    case LOG_WARN: {\
        fprintf(stderr, "%s:%d:%d: WARNING: ", (source), (line), (offset));\
        fprintf(stderr, __VA_ARGS__);\
    } break;\
    case LOG_ERROR: {\
        fprintf(stderr, "%s:%d:%d: ERROR: ", (source), (line), (offset));\
        fprintf(stderr, __VA_ARGS__);\
    } break;\
    default: abort();\
    }\
} while(0)

/** @brief Enumeration for different logging levels that are used in @link bk_log_loc `bk_log_loc`@endlink and @link bk_log `bk_log`@endlink. */
typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} LogLevel;

/**
 * @brief Macro that can push elements into arbitrary dynamic arrays.
 *
 * @param arr Pointer to a struct with `items`, `len` and `cap` fields.
 *            `*arr` can be zero initialized but `items` must be a valid pointer
 *            and must be able to be reallocated with `realloc` if it is non NULL.
 *            `items` is automatically allocated if it is NULL.
 *
 * @param item Assumed to be the same type as `*arr->items`.
*/
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

/**
 * @brief Macro that can be used like `fprintf` but with `String`s.
 *
 * @param str Assumed to be of type `String*`.
 * @param ... `printf` style format arguments.
*/
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
__BK_API bool load_dynamic_schema_loc(const char* file_name, const char* source_file, int source_line);
#define load_dynamic_schema(file_name) load_dynamic_schema_loc(file_name, __FILE__, __LINE__)

/**
 * @brief Function used for obtaining the correct bitfield mask for schema of the specified type in the specified index.
 *
 * @param schema_type Denotes whether the provided schema is static or dynamic.
 * @param index Index of this schema in its corresponding array.
 * @return Returns the bitfield mask that can be used in `CCompund::derived_schemas`.
*/
__BK_API int get_schema_derive(SchemaType schema_type, size_t index);

// Cleanup functions
/** @brief free's the contents of a `CCompound` */
__BK_API void free_ccompund(CCompound cc);
/** @brief free's the contents of a `Field` */
__BK_API void free_field(Field f);
/** @brief free's the contents of a `CType` */
__BK_API void free_ctype(CType ct);
/** @brief free's the contents of a `Entry` */
__BK_API void free_entry(Entry e);

/**
 * @defgroup cparse Parsing C code
 * @brief All definitions and declarations related to parsing C code.
 * @addtogroup cparse
 * @{
*/

/** @brief Kind tag for `Word` */
typedef enum {
    WORD_TOK,
    WORD_ID,
    WORD_END
} WordKind;

/**
 * @brief Tagged union that is used to refer to identifiers or tokens in the same type.
 *
 * This type is mainly for use in @link peek_c `peek_c`@endlink.
*/
typedef struct {
    WordKind kind;
    union {
        int tok;
        const char* id;
    };
} Word;

/**
 * @brief Macro that is used to generate identifier parameters for @link peek_c `peek_c`@endlink.
 *
 * @param s Assumed to be of type `const char*`. This is the string representation of the identifier.
 * @return Expands to a `Word` instance of kind @link WordKind `WordKind::WORD_ID`@endlink.
*/
#define ID(s) (Word){.kind = WORD_ID, .id=s}

/**
 * @brief Macro that is used to generate token parameters for @link peek_c `peek_c`@endlink.
 *
 * @param t Assumed to be of type `int`. This is the integer value of the token.
 * @return Expands to a `Word` instance of kind @link WordKind `WordKind::WORD_TOK`@endlink.
*/
#define TK(t) (Word){.kind = WORD_TOK, .tok=t}

/**
 * @brief Macro that is used to generate the terminator for the parameter list of @link peek_c `peek_c`@endlink.
 *
 * @return Expands to a `Word` instance of kind @link WordKind `WordKind::WORD_END`@endlink.
*/
#define WEND (Word){.kind = WORD_END}

/** @cond */
__BK_API bool va_get_expect_ids(stb_lexer* lex, va_list args);
__BK_API bool va_get_expect_tokens(stb_lexer* lex, va_list args);
__BK_API bool va_get_expect_c(stb_lexer* lex, va_list args);
/** @endcond */

/**
 * @brief Variadic function that looks forward to see if the source file matches the provided identifiers.
 *
 * This macro also returns `false` if it encounters non-identifier tokens.
 *
 * @param lex Pointer to the stb_lexer instance that contains the tokens.
 * @param ... indetifiers of type `const char*` to check for. These parameters should end with NULL.
 * @return Returns `true` if the source file matched the provided indetifiers, `false` otherwise.
*/
__BK_API bool peek_ids(stb_lexer* lex, ...);

/**
 * @brief Variadic function that looks forward to see if the source file matches the provided tokens.
 *
 * @param lex Pointer to the stb_lexer instance that contains the tokens.
 * @param ... tokens of type `int` to check for. These parameters should end with -1.
 * @return Returns `true` if the source file matched the provided tokens, `false` otherwise.
*/
__BK_API bool peek_tokens(stb_lexer* lex, ...);

/**
 * @brief Variadic function that looks forward to see if the source file matches the provided `Word`s.
 *
 * Used in combination with the @link ID `ID`,@endlink @link TK `TK`,@endlink and @link WEND `WEND`@endlink macros for its parameters.
 *
 * @param lex Pointer to the stb_lexer instance that contains the tokens.
 * @param ... `Word`s to check for. These parameters should end with a `Word` of kind @link WordKind `WordKind::WORD_END` @endlink
 * @return Returns `true` if the source file matched the provided `Word`s, `false` otherwise.
*/
__BK_API bool peek_c(stb_lexer* lex, ...);

/**
 * @brief Variadic function that gets tokens from the source file and compares them against the provided tokens.
 *
 * @param lex Pointer to the stb_lexer instance that contains the tokens.
 * @param ... tokens of type `int` to check for. These parameters should end with -1.
 * @return Returns `true` if the source file matched the provided tokens, `false` otherwise.
*/
__BK_API bool get_expect_tokens(stb_lexer* lex, ...);

/**
 * @brief Variadic function that gets identifiers from the source file and compares them against the provided identifiers.
 *
 * This macro also returns `false` if it encounters non-identifier tokens.
 *
 * @param lex Pointer to the stb_lexer instance that contains the tokens.
 * @param ... indetifiers of type `const char*` to check for. These parameters should end with NULL.
 * @return Returns `true` if the source file matched the provided indetifiers, `false` otherwise.
*/
__BK_API bool get_expect_ids(stb_lexer* lex,...);

/** @cond */
bool get_expect_c(stb_lexer* lex,...); // Unused for now
/** @endcond */

/**
 * @brief Parses the provided file and appends all types it could find to the provided `CCompounds` dynamic array.
 *
 * This function doesn't return on errors and instead just logs them and continues to try parsing.
 *
 * @param file_name The name of the currently analyzed file. Used for error messages.
 * @param schemas Dynamic array that contains all loaded schemas.
 * @param content Contents of the source file.
 * @param out Dynamic array for appending analyzed types.
 * @param derive_all Whether the `BkConfig::derive_all` option is on or not.
*/
__BK_API void analyze_file(const char* file_name, String content, CCompounds* out, bool derive_all);
/** @} */

/**
 * @defgroup commandline Command Line
 * @brief All definitions and declarations related to parsing commandline arguments and executing commands.
*/

/**
 * @ingroup commandline
 * @brief Parses a list from the command line.
 *
 * Users are meant to call this function in a loop until its return value equals to NULL.
 *
 * @param src Array of command line arguments, equal to the `argv` from `main`.
 * @param sep Seperator character for the list.
 * @param entry_len Out parameter that is assigned to the current entry's length.
 * @return NULL when there are no more entries in the list or the list couldn't be parsed.
 *         Otherwise, returns a *sized string*. This string is **not null terminated**.
*/
__BK_API char* parse_list(char** src, char sep, size_t* entry_len);

/** @cond */
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
}
/** @endcond */

#define BK_DUMP_UPPER "DUMP"
#define BK_PARSE_UPPER "PARSE"
#define BK_DUMP_LOWER "dump"
#define BK_PARSE_LOWER "parse"

/**
 * @defgroup codegen Code Generation
 * @brief Code generation functions.
 *
 * @addtogroup codegen
 * @{
*/
/** */

/**
 * @brief Generates 'prelude' code that will be generated once at the top of each generated file.
 *
 * For more information, see `StaticSchema::gen_prelude`.
 *
*/
void gen_prelude(String* book_buf, CCompound* ty);

/**
 * @brief Generates the declarations of "dump" functions for the specified type.
 *
 * For more information, see `StaticSchema::gen_dump_decl`.
 *
*/
size_t gen_dump_decl(String* book_buf, CCompound* ty, const char* dst_type);

/**
 * @brief Generates the declarations of "parse" functions for the specified type.
 *
 * For more information, see `StaticSchema::gen_parse_decl`.
 *
*/
size_t gen_parse_decl(String* book_buf, CCompound* ty);

/**
 * @brief Generates the implementations of "dump" functions for the specified type.
 *
 * For more information, see `StaticSchema::gen_dump_impl`.
 *
*/
void gen_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);

/**
 * @brief Generates the implementations of "parse" functions for the specified type.
 *
 * For more information, see `StaticSchema::gen_parse_impl`.
 *
*/
void gen_parse_impl(String* book_buf, CCompound* ty);

/**
 * @brief Interpretes all loaded dynamic schema code for the specified type.
 *
 * To learn about the dynamic schema syntax, see the 'Dynamic Schema' section of the User Documentation.
 *
 * @param book_buf The string buffer that is used to store generated code. The @link print_string `print_string` @endlink
 *        macro available to both this source file and extension authors can be used to output into this buffer.
 * @param ty The type that this function is meant to generate code for, contains all necessary info for code generation.
 * @param dst_type The type of the 'dst' parameter that is meant to be used for generated "dump" functions.
 * @param fmt_macro The name of the macro that is meant to be used **inside generated code** to output into the 'dst' buffer.
*/
void gen_dynamic(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);
/** @} */

/**
 * @defgroup jsoncodegen JSON Schema Code
 * @brief All definitions and declarations related to the included JSON `StaticSchema`.
 * @addtogroup jsoncodegen
 * @{
*/

void gen_json_prelude(String* book_buf);
size_t gen_json_dump_decl(String* book_buf, CCompound* ty, const char* dst_type);
size_t gen_json_parse_decl(String* book_buf, CCompound* ty);
void gen_json_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);
void gen_json_parse_impl(String* book_buf, CCompound* ty);
/** @} */

/**
 * @defgroup debugcodegen Debug Schema Code
 * @brief All definitions and declarations related to the included Debug `StaticSchema`.
 * @addtogroup debugcodegen
 * @{
*/
size_t gen_debug_dump_decl(String* book_buf, CCompound* ty, const char* dst_type);
void gen_debug_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro);
/** @} */

/**
 * @defgroup generatedcode Generated Code
 * @brief Code that was generated using `bk`.
 *
 * Generated with dynamic schema './examples/bkconf.schema' in the main repository.
 *
 * @addtogroup generatedcode
 * @{
*/
int parse_bkconf_BkConfig(const char* src, unsigned long len, BkConfig* dst);
/** @} */

/** @brief Defines different output modes. */
typedef enum {
    /** @brief Mirrors the original file structure. Places generated files next to their source files. */
    O_MIRROR,

    /** @brief Places all generated files inside the provided `output-directory`. */
    O_DIR,

    // O_FILE // TODO: implement optional single-file output
} OutputMode;

/**
 * @ingroup commandline
 * @brief Defines a command that can be executed from the commandline.
*/
typedef struct {
    /** @brief The name of this command displayed in help menus. (like 'help') */
    char* name;

    /** @brief The flag used to invoke this command. (like '-h') */
    char* flag;

    /** @brief The usage text for this command. (like '-h <command (optional)>') */
    char* usage;

    /** @brief The explanatory description of this command that is displayed in help menus. */
    char* desc;

    /**
     * @brief The implementation of this command.
     *
     * @param i Pointer to the index counter that is used to iterate over command line arguments.
     *          `Command`s are expected to increment this counter only if the comamnd accepts argument(s).
     *          So, a flag like '--silent' _shouldn't_ increment this counter at all.
     * @param argc The amount of command line arguments, equal to the `argc` from `main`.
     * @param argv Array of command line arguments, equal to the `argv` from `main`.
    */
    bool (*exec_c)(int* i, int argc, char** argv);
} Command;

// If only we had closures in C...
/** @addtogroup commandline
 * The *_cmd functions are assigned to the `Command::exec_c` field of their respecive `Command`s.
 *  @{
*/
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
bool include_schema_cmd(int* i, int argc, char** argv);
bool silent_cmd(int* i, int argc, char** argv);
bool verbose_cmd(int* i, int argc, char** argv);
bool enable_warn_cmd(int* i, int argc, char** argv);
bool disable_warn_cmd(int* i, int argc, char** argv);
bool derive_all_cmd(int* i, int argc, char** argv);
bool derive_cmd(int* i, int argc, char** argv);
bool disable_dump_cmd(int* i, int argc, char** argv);
bool disable_parse_cmd(int* i, int argc, char** argv);
bool disabled_cmd(int* i, int argc, char** argv);
bool gen_impl_cmd(int* i, int argc, char** argv);
bool gen_fmt_dst_cmd(int* i, int argc, char** argv);
bool gen_fmt_cmd(int* i, int argc, char** argv);
bool offset_type_cmd(int* i, int argc, char** argv);
bool disable_prefix_cmd(int* i, int argc, char** argv);
bool enable_prefix_cmd(int* i, int argc, char** argv);
/** @} */

/**
 * @brief Helper macro that does all necessary checks before executing a command. It also prints a usage message if the comamnd fails.
 *
 * @return Returns `true` if the command succeeded, `false` otherwise.
*/
#define exec_cmd(cmd)\
((cmd)->exec_c ? ((cmd)->exec_c(&i, argc, argv) ? true : (bk_printf("Usage of '%s': %s\n", (cmd)->name, (cmd)->usage), false)) : false)

#define WARN_NO_INCLUDE "no-include"
#define WARN_NO_OUTPUT "no-output"
#define WARN_UNKNOWN_ATTR "unknown-attr"
#define WARN_LIST WARN_NO_INCLUDE"|"WARN_NO_OUTPUT"|"WARN_UNKNOWN_ATTR

/**
 * @var commands
 * Global static array that includes all commands.
*/
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
        .desc = "Changes the path that will be used to load the configuration file (default value is './bk.conf')",
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
        .name = "include-schema",
        .flag = "-is",
        .usage = "-is <schema-file>",
        .desc = "Includes a dynamic schema file",
        .exec_c = include_schema_cmd
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
        .name = "derive",
        .flag = "--derive",
        .usage = "--derive <schema>",
        .desc = "Derives the provided schema for all analyzed structs",
        .exec_c = derive_cmd
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
    // {
    //     .name = "disabled",
    //     .flag = "--disabled",
    //     .usage = "--disabled",
    //     .desc = "Disables all functionality by default. They can be enabled gradually in code with `ENABLE` macros.",
    //     .exec_c = disabled_cmd
    // },
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
static const size_t commands_count = sizeof commands / sizeof *commands;

#define bk_printf(...) (bk.conf.silent ? 0 : printf(__VA_ARGS__))

static char* config_path = "./.bk.conf";
#define BK_FILE_EXT ".bk.h"
#define BK_FILE_EXT_LEN 5

/** @brief Macro that goto's to the cleanup label before returning inside `main` */
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

    StaticSchema json = {
        .gen_prelude = gen_json_prelude,
        .gen_dump_decl = gen_json_dump_decl, 
        .gen_parse_decl = gen_json_parse_decl, 
        .gen_dump_impl = gen_json_dump_impl, 
        .gen_parse_impl = gen_json_parse_impl, 
        .derive_attr = "derive_json",
        .name = "json"
    };
    StaticSchema debug = {
        .gen_prelude = NULL,
        .gen_dump_decl = gen_debug_dump_decl, 
        .gen_parse_decl = NULL, 
        .gen_dump_impl = gen_debug_dump_impl, 
        .gen_parse_impl = NULL, 
        .derive_attr = "derive_debug",
        .name = "debug"
    };
    push_da(&bk.schemas, json);
    push_da(&bk.schemas, debug);

    // For static schema extensions
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

    if (bk.conf.schema_files != NULL) {
        size_t list_len = strlen(bk.conf.schema_files);
        if (list_len == 0) {
            bk_log(LOG_ERROR, "Schema file list in '%s' is empty.\n", config_path);
            ret_clean(1);
        }
        if (bk.conf.schema_files[0] == ',') {
            bk_log(LOG_ERROR, "Schema file list in '%s' starts with comma (',')\n", config_path);
            ret_clean(1);
        }
        char* cursor = bk.conf.schema_files;
        size_t entry_len = 0;

        for (char* ent; (ent = parse_list(&cursor, ',', &entry_len));) {
            if (!load_dynamic_schema(tfmt("%.*s", (int)entry_len, ent))) {
                ret_clean(1);
            }
        }
    }


    if (bk.conf.include_dir == NULL && bk.entries.len == 0) {
        if (bk.conf.warn_no_include) bk_log(LOG_WARN, "No files were included. [-W "WARN_NO_INCLUDE"]\n");
    }
    if (bk.conf.output_dir == NULL) {
        if (bk.conf.warn_no_output) bk_log(LOG_WARN, "No output path set. [-W "WARN_NO_OUTPUT"]\n");
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
        for (size_t i = 0; i < bk.dynamic_schemas.len; ++i) {
            print_string(&book_buf, "#define "SV_FMT"(...)\n", SV_ARG(bk.dynamic_schemas.items[i].derive_attr));
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
                        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
                        .sys_modif = s.st_mtimespec.tv_sec,
                        #else
                        .sys_modif = s.st_mtim.tv_sec,
                        #endif
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
                #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
                in_file->sys_modif = s.st_mtimespec.tv_sec;
                #else
                in_file->sys_modif = s.st_mtim.tv_sec;
                #endif
            }
            if (in_file->sys_modif > in_file->last_analyzed) {
                unsigned long in_hash = djb2(in_file->full);
                bk_log(LOG_INFO, "Analyzing file: %s\n", in_file->name);
                file_buf.len = 0;
                if (!read_entire_file(in_file->full, &file_buf)) continue;

                types.len = 0;
                analyze_file(in_file->name, file_buf, &types, bk.conf.derive_all);
                bk_log(LOG_INFO, "Analayzed %lu type(s).\n", types.len);
                for (size_t i = 0; i < types.len; ++i) {
                    push_da(&all_types, types.items[i]);
                };
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
                    size_t len_before_decls = book_buf.len;
                    for (size_t i = 0; i < types.len; ++i) {
                        gen_prelude(&book_buf, types.items + i);
                        if (!bk.conf.disable_dump) {
                            num_decls += gen_dump_decl(&book_buf, types.items + i, bk.conf.gen_fmt_dst_macro);
                        }
                        if (!bk.conf.disable_parse) {
                            num_decls += gen_parse_decl(&book_buf, types.items + i);
                        }
                    }
                    if (num_decls > 0 || bk.dynamic_schemas.len > 0) {
                        if (num_decls > 0) {
                            print_string(&book_buf, "\n#ifdef %s\n", bk.conf.gen_implementation_macro);
                            for (size_t i = 0; i < types.len; ++i) {
                                if (!bk.conf.disable_dump) {
                                    gen_dump_impl(&book_buf, types.items + i, bk.conf.gen_fmt_dst_macro, bk.conf.gen_fmt_macro);
                                }
                                if (!bk.conf.disable_parse) {
                                    gen_parse_impl(&book_buf, types.items + i);
                                }
                                print_string(&book_buf, "\n#define ___BK_INCLUDE_TYPE_%s\n", types.items[i].name);
                            }
                            print_string(&book_buf, "\n#endif // %s\n", bk.conf.gen_implementation_macro);
                        } else {
                            book_buf.len = len_before_decls;
                        }
                        for (size_t i = 0; i < types.len; ++i) {
                            gen_dynamic(&book_buf, types.items + i, bk.conf.gen_fmt_dst_macro, bk.conf.gen_fmt_macro);
                        }

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
            // print_string(&book_buf, "#ifndef __GENERICS_H__\n");
            // print_string(&book_buf, "#define __GENERICS_H__\n");
            for (size_t i = 0; i < all_types.len; ++i) {
                const char* t_name = all_types.items[i].name;
                print_string(&book_buf, "#ifdef ___BK_IF_TYPE_%s\n", t_name);
                print_string(&book_buf, "#undef ___BK_IF_TYPE_%s\n", t_name);
                print_string(&book_buf, "#endif // ___BK_IF_TYPE_%s\n", t_name);
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
                print_string(&book_buf, "#ifdef ___BK_GENERIC_"BK_DUMP_UPPER"_%s_CASES\n", s_name);
                print_string(&book_buf, "#undef ___BK_GENERIC_"BK_DUMP_UPPER"_%s_CASES\n", s_name);
                print_string(&book_buf, "#endif // ___BK_GENERIC_"BK_DUMP_UPPER"_%s_CASES\n", s_name);
                print_string(&book_buf, "#define ___BK_GENERIC_"BK_DUMP_UPPER"_%s_CASES\\\n", s_name);
                for (size_t j = 0; j < all_types.len; ++j) {
                    const char* t_name = all_types.items[j].name;
                    print_string(&book_buf, "    ___BK_IF_TYPE_%s(%s*: "BK_DUMP_LOWER"_%s_%s)\\\n", t_name, t_name, s_name, t_name);
                }
                print_string(&book_buf, "\n#ifdef "BK_DUMP_LOWER"_%s\n", s_name);
                print_string(&book_buf, "\n#undef "BK_DUMP_LOWER"_%s\n", s_name);
                print_string(&book_buf, "\n#endif // "BK_DUMP_LOWER"_%s\n", s_name);
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
                print_string(&book_buf, "#ifdef ___BK_GENERIC_"BK_PARSE_UPPER"_%s_CASES\n", s_name);
                print_string(&book_buf, "#undef ___BK_GENERIC_"BK_PARSE_UPPER"_%s_CASES\n", s_name);
                print_string(&book_buf, "#endif // ___BK_GENERIC_"BK_PARSE_UPPER"_%s_CASES\n", s_name);
                print_string(&book_buf, "#define ___BK_GENERIC_"BK_PARSE_UPPER"_%s_CASES\\\n", s_name);
                for (size_t j = 0; j < all_types.len; ++j) {
                    const char* t_name = all_types.items[j].name;
                    print_string(&book_buf, "    ___BK_IF_TYPE_%s(%s*: "BK_PARSE_LOWER"_%s_%s)\\\n", t_name, t_name, s_name, t_name);
                }
                print_string(&book_buf, "\n#ifdef "BK_PARSE_LOWER"_%s\n", s_name);
                print_string(&book_buf, "\n#undef "BK_PARSE_LOWER"_%s\n", s_name);
                print_string(&book_buf, "\n#endif // "BK_PARSE_LOWER"_%s\n", s_name);
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
            // print_string(&book_buf, "#endif // __GENERICS_H__\n");
            write_entire_file(tfmt("%s/generics.h", bk.conf.output_dir), &book_buf);
        }
    } while(bk.conf.watch_mode);

    __bk_cleanup:
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

    if (bk.dynamic_schemas.items != NULL) free(bk.dynamic_schemas.items);

    for (size_t i = 0; i < bk.dynamic_source.len; ++i) free(bk.dynamic_source.items[i].items);
    if (bk.dynamic_source.items != NULL) free(bk.dynamic_source.items);
    
    return ret_val;
}

String_View sv(const char* cstr) {
    return (String_View) {
        .items = cstr,
        .len = strlen(cstr),
    };
}

String_View sv_from_parts(const char* items, size_t len) {
    return (String_View) {
        .items = items,
        .len = len,
    };
}

String_View sv_from_str(String* str) {
    return (String_View) {
        .items = str->items,
        .len = str->len,
    };
}

char* sv_to_cstr(String_View sv) {
    char* res = malloc(sv.len + 1);
    memset(res, 0, sv.len + 1);
    sprintf(res, SV_FMT, SV_ARG(sv));
    return res;
}

bool sv_starts_with(String_View sv, String_View prefix) {
    if (sv.len < prefix.len) return false;

    if (strncmp(sv.items, prefix.items, prefix.len) == 0) return true;
    
    return false;
}

String_View sv_chop(String_View sv, size_t n) {
    sv_chop2(&sv, n);
    return sv;
}

bool sv_chop_if_prefix(String_View* sv, String_View prefix) {
    if (sv_starts_with(*sv, prefix)) {
        sv_chop2(sv, prefix.len);
        return true;
    } else {
        return false;
    }
}

String_View sv_chop_line(String_View sv, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        sv = sv_find(sv, '\n');
        sv = sv_chop(sv, 1);
    }
    return sv;
}

void sv_chop2(String_View* sv, size_t n) {
    size_t cn = n <= sv->len ? n : sv->len;

    sv->items += cn;
    sv->len -= cn;
}

String_View sv_trim_start(String_View sv, char c) {
    String_View res = sv;
    for (size_t i = 0; i < sv.len; ++i) {
        if (sv.items[i] == c) {
            sv_chop2(&res, 1);
        } else {
            break;
        }
    }
    return res;
}

String_View sv_trim2_start(String_View sv, String_View cs) {
    String_View res = sv;
    for (size_t i = 0; i < sv.len; ++i) {
        bool found = false;
        for (size_t j = 0; j < cs.len; ++j) {
            if (sv.items[i] == cs.items[j]) {
                found = true;
                break;
            }
        }
        if (found) {
            sv_chop2(&res, 1);
        } else {
            break;
        }
    }
    return res;
}

String_View sv_trim_whitespace_start(String_View sv_) {
    return sv_trim2_start(sv_, sv(" \t\n"));
}

String_View sv_trim_whitespace_end(String_View sv) {
    if (sv.len == 0) return sv;
    for (;;) {
        char c = sv.items[sv.len - 1];
        if (
        (c == ' ') ||
        (c == '\n') ||
        (c == '\t')
        ) {
            --sv.len;
        } else {
            break;
        }
    }
    return sv;
}

String_View sv_trim_whitespace(String_View sv) {
    return sv_trim_whitespace_end(sv_trim_whitespace_start(sv));
}

String_View sv_find(String_View sv, char c) {
    String_View res = sv;
    for (size_t i = 0; i < sv.len; ++i) {
        if (sv.items[i] != c) {
            sv_chop2(&res, 1);
        } else {
            break;
        }
    }
    return res;
}

String_View sv_substr(String_View sv, size_t start, size_t len) {
    // TODO: bounds checks
    return (String_View) {
        .items = sv.items + start,
        .len = len
    };
}

void sv_loc(String_View file, String_View cursor, int* line, int* offset) {
    *line = 1;
    *offset = 1;
    for (size_t i = 0; i < (file.len - cursor.len); ++i) {
        if (file.items[i] == '\n') {
            *line += 1;
            *offset = 1;
        } else {
            *offset += 1;
        }
    }
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
    #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
    out->sys_modif = s.st_mtimespec.tv_sec;
    #else
    out->sys_modif = s.st_mtim.tv_sec;
    #endif
    out->last_analyzed = 0;

    return true;
}

__BK_API bool load_dynamic_schema_loc(const char* file_name, const char* source_file, int source_line) {
    String source = {0}; // alloc
    if (!read_entire_file_loc(file_name, &source, source_file, source_line)) return false;

    String_View schema_source = sv_from_str(&source);
    String_View cursor = sv_trim_whitespace_start(schema_source);

    String_View name_prefix = sv("name:");
    String_View derive_prefix = sv("derive:");
    if (sv_starts_with(cursor, name_prefix)) {
        cursor = sv_find(cursor, '"');
        cursor = sv_chop(cursor, 1);
        String_View name_end = sv_find(cursor, '"');
        String_View name = sv_substr(cursor, 0, cursor.len - name_end.len);
        cursor = name_end;
        cursor = sv_chop(cursor, 1);
        if (cursor.items[0] != ',') {
            int line, offset;
            sv_loc(schema_source, cursor, &line, &offset);
            bk_diag_loc(LOG_ERROR, file_name, line, offset, "Expected comma (',')\n");
            free(source.items);
            return false;
        }
        cursor = sv_chop(cursor, 1);
        cursor = sv_trim_whitespace_start(cursor);
        if (!sv_starts_with(cursor, derive_prefix)) {
            int line, offset;
            sv_loc(schema_source, cursor, &line, &offset);
            bk_diag_loc(LOG_ERROR, file_name, line, offset, "Expected `derive` field\n");
            free(source.items);
            return false;
        }
        cursor = sv_chop(cursor, derive_prefix.len);
        cursor = sv_find(cursor, '"');
        cursor = sv_chop(cursor, 1);
        String_View derive_end = sv_find(cursor, '"');
        String_View derive_attr = sv_substr(cursor, 0, cursor.len - derive_end.len);
        cursor = derive_end;
        cursor = sv_chop(cursor, 1);
        cursor = sv_trim_whitespace_start(cursor);
        bk_log(LOG_INFO, "Loading dynamic schema '"SV_FMT"' with derive attribute '"SV_FMT"'\n", SV_ARG(name), SV_ARG(derive_attr));
        DynamicSchema s = {
            .name = name,
            .derive_attr = derive_attr,
            .source = cursor,
        };
        push_da(&bk.dynamic_schemas, s);
        push_da(&bk.dynamic_source, source);
    } else {
        int line, offset;
        sv_loc(schema_source, cursor, &line, &offset);
        bk_diag_loc(LOG_ERROR, file_name, line, offset, "Expected `name` field\n");
        free(source.items);
        return false;
    }
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

__BK_API void analyze_file(const char* file_name, String content, CCompounds* out, bool derive_all) {
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
                        bk_diag(LOG_INFO, "Unknown type while parsing sruct, skipping...\n");
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
                    bk_diag(LOG_INFO, "Couldn't parse field in struct, skipping...\n");
                }
                stb_c_lexer_get_token(&lex); // name
                field.name = strdup(lex.string); // alloc
                if (!get_expect_tokens(&lex, ';', -1)) {
                    // At this point we don't know if we are looking at the right kind of field. So it doesn't make sense to report this.
                    // bk_diag(LOG_ERROR, "Expected ';'.\n");
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
                    for (size_t i = 0; i < bk.schemas.len; ++i) {
                        StaticSchema* schema = bk.schemas.items + i;
                        if (peek_ids(&lex, schema->derive_attr, NULL)) {
                            strct.derived_schemas |= get_schema_derive(SCHEMA_STATIC, i);
                            matched = true;
                            break;
                        }
                    }
                    if (!matched) {
                        for (size_t i = 0; i < bk.dynamic_schemas.len; ++i) {
                            DynamicSchema* schema = bk.dynamic_schemas.items + i;
                            char* derive_attr = sv_to_cstr(schema->derive_attr);
                            if (peek_ids(&lex, derive_attr, NULL)) {
                                strct.derived_schemas |= get_schema_derive(SCHEMA_DYNAMIC, i);
                                matched = true;
                                free(derive_attr);
                                break;
                            } else {
                                free(derive_attr);
                            }
                        }
                    }
                }

                stb_c_lexer_get_token(&lex); // derive_schema
                if (!matched) {
                    if (bk.conf.warn_unknown_attr) bk_diag(LOG_WARN, "Found unknown attribute '%s' while parsing type '%s'. [-W "WARN_UNKNOWN_ATTR"]\n", lex.string, strct.name);
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

__BK_API int get_schema_derive(SchemaType schema_type, size_t index) {
    switch (schema_type) {
    case SCHEMA_STATIC: {
        return (1 << index);
    } break;
    case SCHEMA_DYNAMIC: {
        return (1 << (index + bk.schemas.len));
    } break;
    }
}

void gen_prelude(String* book_buf, CCompound* ty) {
    for (size_t i = 0; i < bk.schemas.len; ++i) {
        StaticSchema* s = bk.schemas.items + i;
        if (ty->derived_schemas & get_schema_derive(SCHEMA_STATIC, i)) {
            print_string(book_buf, "#ifndef ___BK_PRELUDE_%s___\n", s->name);
            print_string(book_buf, "#define ___BK_PRELUDE_%s___\n", s->name);
            if (s->gen_prelude != NULL) s->gen_prelude(book_buf);
            print_string(book_buf, "#endif // ___BK_PRELUDE_%s___\n", s->name);
        }
    }
}

size_t gen_dump_decl(String* book_buf, CCompound* ty, const char* dst_type) {
    size_t count = 0;
    if (ty->derived_schemas == 0) return count;
    gen_def_guard(BK_DUMP_UPPER);
    print_string(book_buf, "\n");
    for (size_t i = 0; i < bk.schemas.len; ++i) {
        StaticSchema* schema = bk.schemas.items + i;
        if (ty->derived_schemas & get_schema_derive(SCHEMA_STATIC, i)) {
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

size_t gen_parse_decl(String* book_buf, CCompound* ty) {
    size_t count = 0;
    if (ty->derived_schemas == 0) return count;
    gen_def_guard(BK_PARSE_UPPER);
    print_string(book_buf, "\n");
    for (size_t i = 0; i < bk.schemas.len; ++i) {
        StaticSchema* schema = bk.schemas.items + i;
        if (ty->derived_schemas & get_schema_derive(SCHEMA_STATIC, i)) {
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

void gen_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro) {
    if (ty->derived_schemas == 0) return;
    gen_def_guard(BK_DUMP_UPPER);
    print_string(book_buf, "\n");
    for (size_t i = 0; i < bk.schemas.len; ++i) {
        StaticSchema* schema = bk.schemas.items + i;
        if (ty->derived_schemas & get_schema_derive(SCHEMA_STATIC, i)) {
            if (schema->gen_dump_impl != NULL) {
                gen_def_type_guard(BK_DUMP_UPPER);
                schema->gen_dump_impl(book_buf, ty, dst_type, fmt_macro);
                gen_endif_type_guard(BK_DUMP_UPPER);
            }
        }
    }
    gen_endif_guard(BK_DUMP_UPPER);
}

void gen_parse_impl(String* book_buf, CCompound* ty) {
    if (ty->derived_schemas == 0) return;
    gen_def_guard(BK_PARSE_UPPER);
    print_string(book_buf, "\n");
    for (size_t i = 0; i < bk.schemas.len; ++i) {
        StaticSchema* schema = bk.schemas.items + i;
        if (ty->derived_schemas & get_schema_derive(SCHEMA_STATIC, i)) {
            if (schema->gen_parse_impl != NULL) {
                gen_def_type_guard(BK_PARSE_UPPER);
                schema->gen_parse_impl(book_buf, ty);
                gen_endif_type_guard(BK_PARSE_UPPER);
            }
        }
    }
    gen_endif_guard(BK_PARSE_UPPER);
}

void gen_dynamic(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro) {
    // Honestly the current implementation of this is a mess, this function should be cleaned up.
    if (ty->derived_schemas == 0) return;
    print_string(book_buf, "\n#ifndef %s%s\n", bk.conf.disable_macro_prefix, ty->name);
    for (size_t schema_i = 0; schema_i < bk.dynamic_schemas.len; ++schema_i) {
        String impl = {0}; // alloc
        DynamicSchema* schema = bk.dynamic_schemas.items + schema_i;
        print_string(book_buf, "\n#ifndef %s"SV_FMT"\n", bk.conf.disable_macro_prefix, SV_ARG(schema->name));
        String_View cursor = sv_trim_whitespace_start(schema->source);
        bool in_special = false;
        bool in_loop = false;
        bool in_if = false;
        String_View current_type_name = {0};
        String_View special_start = cursor;
        String_View normal_start = cursor;
        String_View special_prefix = sv("$");
        while (cursor.len > 0) {
            if (sv_starts_with(cursor, special_prefix)) {
                if (in_special) {
                    in_special = false;
                    String_View special = sv_substr(special_start, 0, special_start.len - cursor.len);
                    special = sv_trim_whitespace_start(special);
                    if (sv_chop_if_prefix(&special, sv("ty"))) {
                        if (in_loop) {
                            print_string(&impl, "%s", ty->name);
                        } else {
                            print_string(book_buf, "%s", ty->name);
                        }
                    } else if (sv_chop_if_prefix(&special, sv("implguard"))) {
                        print_string(book_buf, "\n#ifdef %s\n", bk.conf.gen_implementation_macro);
                    } else if (sv_chop_if_prefix(&special, sv("endimplguard"))) {
                        print_string(book_buf, "\n#endif // %s\n", bk.conf.gen_implementation_macro);
                    } else if (sv_chop_if_prefix(&special, sv("dumpguard"))) {
                        print_string(book_buf, "\n#ifndef %s"BK_DUMP_UPPER"\n", bk.conf.disable_macro_prefix);
                        print_string(book_buf, "\n#ifndef %s%s_"BK_DUMP_UPPER"\n", bk.conf.disable_macro_prefix, ty->name);
                    } else if (sv_chop_if_prefix(&special, sv("enddumpguard"))) {
                        print_string(book_buf, "\n#endif // %s%s_"BK_DUMP_UPPER"\n", bk.conf.disable_macro_prefix, ty->name);
                        print_string(book_buf, "\n#endif // %s"BK_DUMP_UPPER"\n", bk.conf.disable_macro_prefix);
                    } else if (sv_chop_if_prefix(&special, sv("parseguard"))) {
                        print_string(book_buf, "\n#ifndef %s"BK_PARSE_UPPER"\n", bk.conf.disable_macro_prefix);
                        print_string(book_buf, "\n#ifndef %s%s_"BK_PARSE_UPPER"\n", bk.conf.disable_macro_prefix, ty->name);
                    } else if (sv_chop_if_prefix(&special, sv("endparseguard"))) {
                        print_string(book_buf, "\n#endif // %s%s_"BK_PARSE_UPPER"\n", bk.conf.disable_macro_prefix, ty->name);
                        print_string(book_buf, "\n#endif // %s"BK_PARSE_UPPER"\n", bk.conf.disable_macro_prefix);
                    } else if (sv_chop_if_prefix(&special, sv("fmt"))) {
                        if (in_loop) {
                            print_string(&impl, "%s", fmt_macro);
                        } else {
                            print_string(book_buf, "%s", fmt_macro);
                        }
                    } else if (sv_chop_if_prefix(&special, sv("dst"))) {
                        if (in_loop) {
                            print_string(&impl, "%s", dst_type);
                        } else {
                            print_string(book_buf, "%s", dst_type);
                        }
                    } else if (sv_chop_if_prefix(&special, sv("offset"))) {
                        if (in_loop) {
                            print_string(&impl, "%s", bk.conf.offset_type_macro);
                        } else {
                            print_string(book_buf, "%s", bk.conf.offset_type_macro);
                        }
                    } else if (sv_chop_if_prefix(&special, sv("it"))) {
                        if (in_loop) {
                            if (sv_chop_if_prefix(&special, sv(".type"))) {
                                print_string(&impl, SV_FMT"type"SV_FMT, SV_ARG(special_prefix), SV_ARG(special_prefix));
                            } else {
                                print_string(&impl, SV_FMT"field"SV_FMT, SV_ARG(special_prefix), SV_ARG(special_prefix));
                            }
                        } else {
                            bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': 'it' directives can't be used outside of for loops\n", SV_ARG(schema->name));
                            goto __continue_free;
                        }
                    } else if (sv_chop_if_prefix(&special, sv("tag"))) {
                        if (in_loop) {
                            print_string(&impl, SV_FMT"tag"SV_FMT, SV_ARG(special_prefix), SV_ARG(special_prefix));
                        } else {
                            bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': 'tag' directive can't be used outside of for loops\n", SV_ARG(schema->name));
                            goto __continue_free;
                        }
                    } else if (sv_chop_if_prefix(&special, sv("for"))) {
                        if (in_loop) {
                            bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': Nested for loops aren't supported\n", SV_ARG(schema->name));
                            goto __continue_free;
                        } else {
                            if (sv_find(special, '{').items[0] != '{') {
                                bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': Expected '{' in for loop\n", SV_ARG(schema->name));
                                goto __continue_free;
                            } else {
                                in_loop = true;
                            }
                        }
                    } else if (sv_chop_if_prefix(&special, sv("if"))) {
                        if (sv_find(special, '{').items[0] != '{') {
                            bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': Expected '{' in if cond\n", SV_ARG(schema->name));
                            goto __continue_free;
                        } else {
                            in_if = true;
                        }
                        special = sv_trim_whitespace_start(special);
                        String_View cond = sv_substr(special, 0, special.len - sv_find(special, ' ').len);
                        if (sv_starts_with(cond, sv("index")) && in_loop) {
                            cond = special;
                            cond = sv_chop(cond, 5);
                            cond = sv_trim_whitespace_start(cond);
                            String_View op_v = sv_substr(cond, 0, cond.len - sv_find(cond, ' ').len);
                            cond = sv_find(cond, ' ');
                            cond = sv_trim_whitespace_start(cond);
                            String_View value_v = sv_substr(cond, 0, cond.len - sv_find(cond, ' ').len);
                            if (sv_starts_with(op_v, sv("=="))) {
                                print_string(&impl, "$if == "SV_FMT" {$", SV_ARG(value_v));
                            } else if (sv_starts_with(op_v, sv("!="))) {
                                print_string(&impl, "$if != "SV_FMT" {$", SV_ARG(value_v));
                            } else {
                                bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': Unknown binary op '"SV_FMT"' in if cond\n", SV_ARG(schema->name), SV_ARG(op_v));
                                goto __continue_free;
                            }

                        } else {
                            current_type_name = cond;
                            print_string(&impl, "$if "SV_FMT" {$", SV_ARG(current_type_name));
                        }
                    } else if (sv_chop_if_prefix(&special, sv("}"))) {
                        if (in_if) {
                            in_if = false;
                            print_string(&impl, "$}$");
                        } else {
                            if (in_loop) {
                                in_loop = false;
                                for (size_t field_i = 0; field_i < ty->fields.len; ++field_i) {
                                    Field* field = ty->fields.items + field_i;
                                    String_View cursor = sv_from_str(&impl);
                                    String_View special_start = cursor;
                                    String_View normal_start = cursor;
                                    bool in_special = false;
                                    bool in_if = false;
                                    bool in_if_cond_true = false;
                                    bool in_correct_type = false;
                                    // Not doing syntax error checks error since this is generated intermediate code
                                    while (cursor.len > 0) {
                                        if (sv_starts_with(cursor, special_prefix)) {
                                            if (in_special) {
                                                in_special = false;
                                                String_View special = sv_substr(special_start, 0, special_start.len - cursor.len);
                                                special = sv_trim_whitespace_start(special);
                                                if (sv_chop_if_prefix(&special, sv("if"))) {
                                                    in_if = true;
                                                    in_if_cond_true = false;
                                                    in_correct_type = false;
                                                    special = sv_trim_whitespace_start(special);
                                                    String_View cond = sv_substr(special, 0, special.len - sv_find(special, ' ').len);
                                                    if (sv_chop_if_prefix(&cond, sv("=="))) {
                                                        String_View value_v = sv_substr(cond, 0, cond.len - sv_find(cond, ' ').len);
                                                        char* value_c = sv_to_cstr(value_v); // alloc
                                                        size_t value = (size_t)(atoi(value_c));
                                                        if (value == field_i) in_if_cond_true = true;
                                                        free(value_c);
                                                    } else if (sv_chop_if_prefix(&cond, sv("!="))) {
                                                        String_View value_v = sv_substr(cond, 0, cond.len - sv_find(cond, ' ').len);
                                                        char* value_c = sv_to_cstr(value_v); // alloc
                                                        size_t value = (size_t)(atoi(value_c));
                                                        if (value != field_i) in_if_cond_true = true;
                                                        free(value_c);
                                                    } else {
                                                        String_View type_name = cond;
                                                        switch (field->type.kind) {
                                                        case CPRIMITIVE: {
                                                        switch (field->type.type) {
                                                        case CINT: {
                                                            if (sv_starts_with(type_name, sv("CINT"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        case CUINT: {
                                                            if (sv_starts_with(type_name, sv("CUINT"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        case CLONG: {
                                                            if (sv_starts_with(type_name, sv("CLONG"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        case CULONG: {
                                                            if (sv_starts_with(type_name, sv("CULONG"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        case CCHAR: {
                                                            if (sv_starts_with(type_name, sv("CCHAR"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        case CFLOAT: {
                                                            if (sv_starts_with(type_name, sv("CFLOAT"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        case CBOOL: {
                                                            if (sv_starts_with(type_name, sv("CBOOL"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        case CSTRING: {
                                                            if (sv_starts_with(type_name, sv("CSTRING"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        }
                                                        } break;
                                                        case CEXTERNAL: {
                                                            if (sv_starts_with(type_name, sv("CEXTERNAL"))) {
                                                                in_if_cond_true = true;
                                                                in_correct_type = true;
                                                            }
                                                        } break;
                                                        }
                                                    }
                                                } else if (sv_chop_if_prefix(&special, sv("}"))) {
                                                    in_if_cond_true = false;
                                                    in_correct_type = false;
                                                    in_if = false;
                                                } else if (sv_chop_if_prefix(&special, sv("type"))) {
                                                    if (field->type.kind == CEXTERNAL && in_correct_type) {
                                                        if ((in_if && in_correct_type) || !in_if)
                                                            print_string(book_buf, "%s", field->type.name);
                                                    } else if (in_correct_type) {
                                                        bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': $it.type$ can only be used in CEXTERNAL fields.\n", SV_ARG(schema->name));
                                                        goto __continue_free;
                                                    }
                                                } else if (sv_chop_if_prefix(&special, sv("tag"))) {
                                                    if ((in_if && in_if_cond_true) || !in_if)
                                                        print_string(book_buf, "%s", field->tag ? field->tag : field->name);
                                                } else if (sv_chop_if_prefix(&special, sv("field"))) {
                                                    if ((in_if && in_if_cond_true) || !in_if)
                                                        print_string(book_buf, "%s", field->name);
                                                } else {
                                                    bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': '"SV_FMT"': INTERNAL ERROR REPORT AS BUG IF ENCOUNTERED\n", SV_ARG(schema->name), SV_ARG(special));
                                                    break;
                                                }
                                                cursor = sv_chop(cursor, special_prefix.len);
                                                normal_start = cursor;
                                            } else {
                                                in_special = true;
                                                if (cursor.len > 1) {
                                                    if ((in_if && in_if_cond_true) || !in_if) {
                                                        String_View v = sv_substr(normal_start, 0, normal_start.len - cursor.len);
                                                        print_string(book_buf, SV_FMT, SV_ARG(sv_trim_whitespace(v)));
                                                    }
                                                }
                                                cursor = sv_chop(cursor, special_prefix.len);
                                                special_start = cursor;
                                            }
                                        } else {
                                            cursor = sv_chop(cursor, 1);
                                        }
                                    }
                                    String_View v = sv_substr(normal_start, 0, normal_start.len - cursor.len);
                                    print_string(book_buf, SV_FMT, SV_ARG(sv_trim_whitespace(v)));
                                }
                                impl.len = 0;                                
                            } else {
                                bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': Mismatched '}'\n", SV_ARG(schema->name));
                                goto __continue_free;
                            }
                        }
                    } else {
                        bk_log(LOG_ERROR, "In dynamic schema '"SV_FMT"': Unknown special directive '"SV_FMT"'\n", SV_ARG(schema->name), SV_ARG(special));
                        goto __continue_free;
                    }
                    cursor = sv_chop(cursor, special_prefix.len);
                    normal_start = cursor;
                } else {
                    in_special = true;
                    if (cursor.len > 1) {
                        if (in_loop) {
                            print_string(&impl, SV_FMT, SV_ARG_N(normal_start, normal_start.len - cursor.len));
                        } else {
                            String_View v = sv_substr(normal_start, 0, normal_start.len - cursor.len);
                            print_string(book_buf, SV_FMT, SV_ARG(v));
                        }
                    }
                    cursor = sv_chop(cursor, special_prefix.len);
                    special_start = cursor;
                }
            } else {
                cursor = sv_chop(cursor, 1);
            }
        }
        String_View post_loop = sv_substr(normal_start, 0, normal_start.len - cursor.len);


        print_string(book_buf, "\n"SV_FMT, SV_ARG(sv_trim_whitespace(post_loop)));

        print_string(book_buf, "\n#endif // %s"SV_FMT"\n", bk.conf.disable_macro_prefix, SV_ARG(schema->name));
        __continue_free:
        free(impl.items);
    }
    print_string(book_buf, "\n#endif // %s%s\n", bk.conf.disable_macro_prefix, ty->name);
}

// JSON generation
void gen_json_prelude(String* book_buf) {
    print_string(book_buf, "typedef enum {\n");
    print_string(book_buf, "    BKJSON_OK = 0,\n");
    print_string(book_buf, "    BKJSON_cJSON_ERROR,\n");
    print_string(book_buf, "    BKJSON_FIELD_NOT_FOUND,\n");
    print_string(book_buf, "    BKJSON_MISMATCHED_FIELD_TYPE,\n");
    print_string(book_buf, "} BkJSON_Result;\n");
}
size_t gen_json_dump_decl(String* book_buf, CCompound* ty, const char* dst_type) {
    print_string(book_buf, "void dump_json_%s(%s* item, %s dst);\n", ty->name, ty->name, dst_type);
    return 1;
}
size_t gen_json_parse_decl(String* book_buf, CCompound* ty) {
    print_string(book_buf, "BkJSON_Result parse_cjson_%s(cJSON* src, %s* dst);\n\n", ty->name, ty->name);
    print_string(book_buf, "/// WARN: Immediately returns on error, so `dst` might be partially filled.\n");
    print_string(book_buf, "BkJSON_Result parse_json_%s(const char* src, unsigned long len, %s* dst);\n", ty->name, ty->name);
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
    print_string(book_buf, "BkJSON_Result parse_cjson_%s(cJSON* src, %s* dst) {\n", ty->name, ty->name);
    print_string(book_buf, "    BkJSON_Result _res = 0; (void)_res;\n");
    for (size_t i = 0; i < ty->fields.len; ++i) {
        Field* f = ty->fields.items + i;
        char* tag = f->tag ? f->tag : f->name;
        print_string(book_buf, "    cJSON* %s_%s = cJSON_GetObjectItemCaseSensitive(src, \"%s\");\n", ty->name, f->name, tag);
        print_string(book_buf, "    if (!%s_%s) return BKJSON_FIELD_NOT_FOUND;\n", ty->name, f->name);
        if (f->type.kind == CEXTERNAL) {
            print_string(book_buf, "    _res = parse_cjson_%s(%s_%s, &dst->%s);\n", f->type.name, ty->name, f->name, f->name);
            print_string(book_buf, "    if (_res) return _res;\n");
        } else if (f->type.kind == CPRIMITIVE) {
            switch (f->type.type) {
            case CINT: case CUINT: {
                print_string(book_buf, "    if (cJSON_IsNumber(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valueint;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return BKJSON_MISMATCHED_FIELD_TYPE;\n");
                print_string(book_buf, "    }\n");
            } break;
            case CLONG: case CULONG: case CFLOAT: {
                print_string(book_buf, "    if (cJSON_IsNumber(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valuedouble;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return BKJSON_MISMATCHED_FIELD_TYPE;\n");
                print_string(book_buf, "    }\n");
            } break;
            case CCHAR: {
                print_string(book_buf, "    if (cJSON_IsString(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        if (!%s_%s->valuestring) { return 1; };\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = *%s_%s->valuestring;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return BKJSON_MISMATCHED_FIELD_TYPE;\n");
                print_string(book_buf, "    }\n");
                    
            } break;
            case CBOOL: {
                print_string(book_buf, "    if (cJSON_IsBool(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = %s_%s->valueint;\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return BKJSON_MISMATCHED_FIELD_TYPE;\n");
                print_string(book_buf, "    }\n");
                    
            } break;
            case CSTRING: {
                print_string(book_buf, "    if (cJSON_IsString(%s_%s)) {\n", ty->name, f->name);
                print_string(book_buf, "        if (!%s_%s->valuestring) { return 1; };\n", ty->name, f->name);
                print_string(book_buf, "        dst->%s = strdup(%s_%s->valuestring);\n", f->name, ty->name, f->name);
                print_string(book_buf, "    } else {\n");
                print_string(book_buf, "        return BKJSON_MISMATCHED_FIELD_TYPE;\n");
                print_string(book_buf, "    }\n");
            } break;
            }
        } else {
            abort();
        }
    }
    print_string(book_buf, "    return BKJSON_OK;\n");
    print_string(book_buf, "}\n");
    print_string(book_buf, "/// WARN: Immediately returns on error, so `dst` might be partially filled.\n");
    print_string(book_buf, "BkJSON_Result parse_json_%s(const char* src, unsigned long len, %s* dst) {\n", ty->name, ty->name);
    print_string(book_buf, "    cJSON* json = cJSON_ParseWithLength(src, len);\n");
    print_string(book_buf, "    if (!json) return BKJSON_cJSON_ERROR;\n");
    print_string(book_buf, "    BkJSON_Result res = parse_cjson_%s(json, dst);\n", ty->name);
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

bool include_schema_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        if (!load_dynamic_schema(argv[*i])) return false;
        return true;
    }
    return false;    
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

bool derive_cmd(int* i, int argc, char** argv) {
    if (++*i < argc) {
        char* name = argv[*i];
        bool found = false;
        for (size_t j = 0; j < bk.schemas.len; ++j) {
            if (strcmp(name, bk.schemas.items[j].name) == 0) {
                bk.derive_schemas |= get_schema_derive(SCHEMA_STATIC, j);
                found = true;
                break;
            }
        }
        if (found) {
            return true;
        }
        if (bk.conf.warn_unknown_attr) bk_log(LOG_WARN, "No schema named '%s' was defined.\n", name);
    }
    return false;    
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

// Generated code, generated with dynamic schema './examples/bkconf.schema'
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
			if (strcmp(str_buf, "output_mode") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->output_mode= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "generics") == 0) {dst->generics= value_bool;}else if (strcmp(str_buf, "silent") == 0) {dst->silent= value_bool;}else if (strcmp(str_buf, "verbose") == 0) {dst->verbose= value_bool;}else if (strcmp(str_buf, "warn_unknown_attr") == 0) {dst->warn_unknown_attr= value_bool;}else if (strcmp(str_buf, "warn_no_include") == 0) {dst->warn_no_include= value_bool;}else if (strcmp(str_buf, "warn_no_output") == 0) {dst->warn_no_output= value_bool;}else if (strcmp(str_buf, "disable_dump") == 0) {dst->disable_dump= value_bool;}else if (strcmp(str_buf, "disable_parse") == 0) {dst->disable_parse= value_bool;}else if (strcmp(str_buf, "disabled_by_default") == 0) {dst->disabled_by_default= value_bool;}else if (strcmp(str_buf, "watch_mode") == 0) {dst->watch_mode= value_bool;}else if (strcmp(str_buf, "watch_delay") == 0) {dst->watch_delay= value_int;}else if (strcmp(str_buf, "gen_fmt_macro") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->gen_fmt_macro= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "gen_implementation_macro") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->gen_implementation_macro= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "gen_fmt_dst_macro") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->gen_fmt_dst_macro= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "offset_type_macro") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->offset_type_macro= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "disable_macro_prefix") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->disable_macro_prefix= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "enable_macro_prefix") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->enable_macro_prefix= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "derive_all") == 0) {dst->derive_all= value_bool;}else if (strcmp(str_buf, "include_dir") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->include_dir= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "include_files") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->include_files= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "schema_files") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->schema_files= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}else if (strcmp(str_buf, "output_dir") == 0) {str_buf[sprintf(str_buf, "%.*s", (int)(value_end - value_start) + 1, value_start)] = 0;
					dst->output_dir= strdup(str_buf);
					str_buf[sprintf(str_buf, "%.*s", (int)(name_end - name_start) + 1, name_start)] = 0;}
			name_start = cur + 1;
		} else if (*cur == '=') {
			name_end = cur - 1;
			value_start = cur + 1;
		}
	}
	return 0;
}

#endif // __BOOKKEEPER_GEN_C__
