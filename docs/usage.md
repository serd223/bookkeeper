# Usage

## Including `bookkeeper` in your project
The recommended way to use `bookkeeper` is to vendor [`bk.c`](../bk.c) and [`stb_c_lexer.h`](https://github.com/nothings/stb/blob/master/stb_c_lexer.h) into your project.

While compiling [`bk.c`](../bk.c), the only thing you need to make sure of is that the compiler should be able to find `stb_c_lexer.h`. Apart from that, no additional flags or linkage is required.

## Basic usage
In order to generate code, `bk` expects input files (either by supplying them one by one, or supplying an include directory) and an output directory. By default, `bk` places generated files next to their source files (mirroring the original file structure). These generated files have the following naming schema: `original_file_name.bk.h`. Special headers like `derives.h` and `generics.h` are always placed inside the output directory. `derives.h` is generated even if no input files were supplied, so you can do an 'empty run' of `bk` only to acquire `derives.h`.

Here is what a standard invocation of `bk` would look like:
```console
  $ bk -i infile1.c -i infile2.h -I ./src/ -o ./gen/
```
Assuming that original file structure is like the following:
```
  infile1.c
  infile2.h
  src/
     |- file1.c
     |- file2.h
     |- file2.c
     |- file3.h
  gen/
```
This would be the file structure after running `bk`:
```
  infile1.c
  infile1.c.bk.h
  infile2.h
  infile2.h.bk.h
  src/
     |- file1.c
     |- file1.c.bk.h
     |- file2.h
     |- file2.h.bk.h
     |- file2.c
     |- file2.c.bk.h
     |- file3.h
     |- file3.h.bk.h
  gen/
     |-dervies.h
```

## Using generated code
Generated functions have the following signatures:
```c
// BK_FMT_DST_t is a redefinable macro defined as FILE* by default
void dump_$schema$_$type$($type$* item, BK_FMT_DST_t dst) {
    /* generated impl */
}

// returns zero if an error happens, otherwise returns 1
int parse_$schema$_$type$(char* src, unsigned long len, $type$* dst) {
    /* generated impl */
}
```
In order to generate functionality for a specific schema, you need to 'derive' that schema. This is how that would look like:
```c
// inside my_struct.c
typedef struct {
    int some_field; tag("SomeField")
    const char* some_other_field;
    /* fields */
} MyStruct derive_json();
```
`derive_*` and `tag` are special 'attribute macros' that allows you to tell bookkeeper what to generate directly from your code. These macros expand to nothing and are located inside the generated `output-directory/derives.h` file, you may acquire this file by doing an 'empty run' with `bk -o <output-directory>`.

The `tag` attribute allows you to customize the generated/expected name of a specific field. In this case, the generated JSON structure will contain a "SomeField" field instead of a "some_field" field. This is also the same for parsing, a "SomeField" field will be expected inside of the JSON source instead of a "some_field" field.

`derive_*` style attributes allow types to 'derive' functionalities for different schemas. You can derive multiple schemas for a struct like:
```c
typedef struct {
  /* fields */
} OtherStruct derive_json() derive_debug();
```
You can also use the `derive_all` attribute to derive all defined schemas (including [extensions](./extensions.md)).

In our case, `MyStruct` derives JSON functionality and will have the following functions available:
```c
// BK_FMT_DST_t is a redefinable macro defined as FILE* by default
void dump_json_MyStruct(MyStruct* item, BK_FMT_DST_t dst) {
    /* generated impl */
}

// returns zero if an error happens, otherwise returns 1
int parse_json_MyStruct(char* src, unsigned long len, MyStruct* dst) {
    /* generated impl */
}
```
These functions can be safely used after including "my_struct.c.bk.h".

Depending on the `output-mode` (see [command line options](#command-line-options)), generated files are either placed next to their 'source' files (`mirror` mode), or inside `output-directory` (`dir` mode). The `derives.h` file is always placed inside `output-directory` regardless of `output-mode`.

The behavior of generated code can be tweaked with defining/redefining certain macros while including them. Although the specific names of these macros can be customized, here are some of them explained with their default names:
  * Dump functions use a macro named `BK_FMT` defined inside the `*.bk.h` files to output into the provided `dst` buffer.
  * The type of this `dst` argument for the 'dump' family of functions depends on the `BK_FMT_DST_t` macro that you should redefine if your `BK_FMT` implementation expects a different type from the default one. The default implementation uses `fprintf` and expects `dst` to be `FILE*` but it can be redefined inside your code before including your `*.bk.h` file. (See [dump_people.c](../examples/dump_people.c))
  * There are also disable macros generated for each type, these macros have a prefix `disable-prefix` which defaults to `BK_DISABLE_`. The following examples will be shown with the default prefix:
    - `BK_DISABLE_DUMP`: Disables dump functionality.
    - `BK_DISABLE_PARSE`: Disables parse functionality.
    - `BK_DISABLE_$type$`: Disables all functionality that belongs to $type$.
    - `BK_DISABLE_$type$_DUMP`: Disables dump functionality that belongs to $type$.
    - `BK_DISABLE_$type$_PARSE`: Disables parse functionality that belongs to $type$.
    - `BK_DISABLE_$type$_$schema$`: Disables all functionality that works on $schema$ and belongs to $type$.
    - `BK_DISABLE_$type$_$schema$_DUMP`: Disables dump functionality that works on $schema$ and belongs to $type$.
    - `BK_DISABLE_$type$_$schema$_PARSE`: Disables parse functionality that works on $schema$ and belongs to $type$.
    - `BK_DISABLE_$schema$`: Disables all functionality that works on $schema$.
    - `BK_DISABLE_$schema$_DUMP`: Disables dump functionality that works on $schema$.
    - `BK_DISABLE_$schema$_PARSE`: Disables parse functionality that works on $schema$.
  * You can also use the `--disabled` flag with `bk` to make all functionality disabled by default. In this mode, you can use the same macros listed above with the `enable-prefix` instead of `disable-prefix`. `enable-prefix` defaults to `BK_ENABLE_`

The names of the special generated macros mentioned above (like `BK_FMT`) can all be customized via [command line options](#command-line-options) or [configuration files](./config.md).

### Dependencies of generated code
Here is a list of dependencies the **generated code** may depend on:
#### parse_json_*
 - These functions depend on the [cJSON](https://github.com/DaveGamble/cJSON) library to parse JSON. Users are expected to have already included this library before including generated code. (See [parse_people.c](../examples/parse_people.c))

## Watch mode
Instead of running `bk` over and over everytime a file changes, you can run `bk` with the `-w` flag to enable watch mode which 'watches' the provided input files for any changes and automatically analyzes and regenerates code for files with recent changes.

>[!WARNING]
> This feature is still work in progress and currently uses too much CPU. There currently exists a customizable `watch-delay` to combat this problem but it is more of a band-aid solution

## Generic macros
By running `bk` with the `--generics` flag, you can generate `output-directory/generics.h` which contains special generics macros (introduced in C11) that call the appropriate function depending on the parameter type. These functions look like this:
```c
#include "my_struct.bk.h"
#include "generics.h"

/* ... */

MyStruct m = { /* init fields */ };
dump_json(&m, stdout); // generic macro
```
In this case, the generic macro call will expand to `dump_json_MyStruct((&m), (stdout))`.

>[!WARNING]
> The current state of generics in `bk` is very much experimental and they don't play well with ENABLE/DISABLE macros. The generated code contains a lot of macro trickery to enable/disable types depending on if they were already included or not. That is necessary because the compiler doesn't just ignore unknown types in generic macros. Since we need to put all analyzed types in a single huge generic macro and we can't expect users to bring all analyzed types into scope just to use generic macros, we resort to preprocessor black magic. Another caveat with the current generics is that they assume that all schemas will respect the `dump/parse_$schema$_$type$` convention of naming their generated functions.

## Advanced usage

>[!WARNING]
> This section is incomplete. TODO: mention disable macros, disabled by default mode, etc.

## Command line options

 * help:
   - Usage: `bk -h <command (optional)>`
   - Description: Prints a list of all commands or information about the provided command

 * config-path:
   - Usage: `--config-path <file>`
   - Description: Changes the path that will be used to load the configuration file (default value is './bk.conf')

 * output-mode:
   - Usage: `-om <mirror|dir>`
   - Description: Sets the preffered output mode. `mirror` puts generated files next to the files they were generated from. `dir` puts all generated files in the specified `output-directory`. (`derives.h` is always placed inside `output-directory`)

 * gen-ext:
   - Usage: `--gen-ext <file> <output path>`
   - Description: Generates the extension header from `bk-source` (bk.c) that contains the definitions that should be included inside static schema extensions.

 * generics:
   - Usage: `--generics`
   - Description: Generates generic macros for dump/parse functions. These macros rely on schemas respecting the `dump/parse_$schema$_$type$` standard. The generic macros will be placed inside `output-directory/generics.h`

 * watch:
   - Usage: `-w`
   - Description: Enables watch mode that constantly analyzes recently modified files with a `watch-delay` second delay. (Exit with `CTRL-C`)

 * watch-delay:
   - Usage: `--watch-delay <integer>`
   - Description: Sets `watch-delay` option, for more information see `watch`.

 * include-file:
   - Usage: `-i <file>`
   - Description: The included file will be analyzed regardless of its extension

 * include-directory:
   - Usage: `-I <directory>`
   - Description: The provided directory will be searched for '.c' or '.h' files to analyze

 * output-directory:
   - Usage: `-o <directory>`
   - Description: All generated files will be placed inside the provided directory

 * schemas:
   - Usage: `--schemas`
   - Description: Displays a list of loaded schemas

 * silent:
   - Usage: `--silent`
   - Description: Disables all terminal output

 * verbose:
   - Usage: `-v`
   - Description: Enables verbose terminal output

 * enable-warning:
   - Usage: `-W <no-include|no-output|unknown-attr>`
   - Description: Enables the specified warning

 * disable-warning:
   - Usage: `-dW <no-include|no-output|unknown-attr>`
   - Description: Disables the specified warning

 * derive-all:
   - Usage: `--derive-all`
   - Description: Derives all possible schemas for all analyzed structs

 * disable-dump:
   - Usage: `--disable-dump`
   - Description: Disables the generation of `dump` functions

 * disable-parse:
   - Usage: `--disable-parse`
   - Description: Disables the generation of `parse` functions

 * disabled:
   - Usage: `--disabled`
   - Description: Disables all functionality by default. They can be enabled gradually in code with `ENABLE` macros.

 * gen-implementation
   - Usage: `--gen-implementation <name>`:
   - Description: Sets the macro that will be used in the generated code to control enabling implementation (`BK_IMPLEMENTATION`)

 * gen-fmt-dst:
   - Usage: `--gen-fmt-dst <name>`
   - Description: Sets the macro that will be used in the generated code to control the type of `dst` in `dump` functions (`BK_FMT_DST_t`)

 * gen-fmt:
   - Usage: `--gen-fmt <name>`
   - Description: Sets the macro that will be used in the generated `dump` functions to output with `printf` style arguments (`BK_FMT`)

 * offset-type:
   - Usage: `--offset-type <name>`
   - Description: Sets the macro that will be used in the generated code to control the type of the `offset` variable inside `dump` functions (`BK_OFFSET_t`)

 * disable-prefix:
   - Usage: `--disable-prefix <name>`
   - Description: Sets the prefix of the generated macros that disable specific stuff, like `$prefix$$type$_$schema$` (`BK_DISABLE_`)

 * enable-prefix:
   - Usage: `--enable-prefix <name>`
   - Description: Sets the prefix of the generated macros that enable specific stuff, like `$prefix$$type$_$schema$` (`BK_ENABLE_`)
