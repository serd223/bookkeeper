# Extensions

## Static Schema Extensions

These extensions directly wrap [`bk.c`](../bk.c) and bake new extensions directly into the executable.

Static schema extensions require two files: [`bk.c`](../bk.c) and the generated 'extension header' (generate with `bk --gen-ext path/to/bk.c bk_ext.h`). Unlike other generated files, you may include the extension header directly in your repo instead of asking users to generate it themselves since the contents of that header only depend on the `bookkeeper` source code. The generated header contains the necessary license, so that isn't a concern.

A simple static extension looks like this: (See the [extension example](../examples/bk_ext.c) for a full example)
```c
// named something like bk_wrap.c
#include "bk_ext.h" // extension header
// Common includes you probably will need in extensions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t gen_example_prelude(String* book_buf) { /* impl */ }
size_t gen_example_dump_decl(String* book_buf, CCompound* ty, const char* dst_type) { /* impl */ }
size_t gen_example_parse_decl(String* book_buf, CCompound* ty) { /* impl */ }
void gen_example_dump_impl(String* book_buf, CCompound* ty, const char* dst_type, const char* fmt_macro) { /* impl */ }
void gen_example_parse_impl(String* book_buf, CCompound* ty) { /* impl */ }

// The `name` field should be unique among other schemas for
// macros that mention schemas to work properly
static StaticSchema example = {
    .gen_prelude = gen_example_prelude
    .gen_dump_decl = gen_example_dump_decl,
    .gen_parse_decl = gen_example_parse_decl,
    .gen_dump_impl = gen_example_dump_impl,
    .gen_parse_impl = gen_example_parse_impl,
    .derive_attr = "derive_example",
    .name = "example"
};
#define BK_ADD_SCHEMAS(s)\
push_da(&s, example);
// This file should be able to find `stb_c_lexer.h` since it is included by `bk.c`
#include "bk.c"
```
The extension header ("bk_ext.h" in this case) contains necessary definitions and declarations that you may need inside of your extension code. See the [`bookkeeper documentation`](../README.md#bookkeeper-documentation) and the provided schemas in [`bk.c`](../bk.c) for more information on how schemas work and the different types, functions and macros schemas use.

After writing your wrapper, you can compile your wrapper code ("bk_wrap.c" in this case) by making sure it can include the extension header, `bk.c` and `stb_c_lexer.h`. Apart from those includes, no additional flags or linkage is required.
