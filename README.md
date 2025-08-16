# bookkeeper
`bookkeeper` is a single source file C code generation tool that generates boilerplate serialization/deserialization code for your user defined structures.

`bookkeeper` is heavily inspired by the Rust [`serde`](https://serde.rs/) crate and [this Tsoding stream](https://youtu.be/hnM6aSpWJ8c?si=7WqJW0dy8oaJtdmm).

# Quick Start
Here is a simple example that outputs both JSON and Debug output: ([quick.c](./examples/quick.c))
```c
#include <stdio.h>

#define BK_IMPLEMENTATION
#define BK_DISABLE_json_PARSE

#include "people.h" // Example file
#include "people.h.bk.h" // Generated file

typedef struct {
    Person friend_info; tag("info")
    int years_known;
} Friend derive_json() derive_debug();
#include "quick.c.bk.h" // Generated file

int main(void) {
    Person p = {.name = "Alice", .age = 30, .married = false};
    Friend f = {.friend_info = p,.years_known = 5};
    printf("JSON:\n");
    dump_json_Friend(&f, stdout);
    printf("\n\nDebug:\n");
    dump_debug_Friend(&f, stdout);
    return 0;
}
```
Here is its output:
```
JSON:
{"info":{"name":"Alice","age":30,"married":false},"years_known":5}

Debug:
Friend {
    friend_info: Person {
        (string) name: Alice
        (int) age: 30
        (bool) married: false
    }
    (int) years_known: 5
}
```
And this is how you can build it:
```console
    $ git clone https://github.com/serd223/bookkeeper.git && cd bookkeeper
    $ mkdir build && mkdir gen
    $ # Uses `clang` by default, set `CC` in the `Makefile` if you prefer another compiler.
    $ make quick
    $ ./build/quick
```

# But why?
As C programmers we find ourselves constantly writing boilerplate code for parsing some config struct, or just simply printing any struct. C lacks the necessary metaprogramming tools to automate the generation of this kind of boilerplate code (unlike more modern languages where you can do `#derive(Debug)` or `deriving Show`). So we either write it by hand every single time (too much work), ask LLMs to write the code for us (unreliable and requires code review), or use code generation tools like `bookkeeper`. `bookkeeper` aims to be an easy to use, extendible, user friendly, embedded friendly and portable solution to this problem.

>[!WARNING]
> Although bookkeeper aims to be all of those things, it is still somewhat early in development and the tool itself only supports Linux. The generated code is mostly embedded friendly and portable, though.

# Overview
The `bk` tool requires an output directory and (optional) input file(s). The output directory can be specified with `-o`. Then you can either supply files one by one with the `-i` flag (like `-i file1 -i file`), or you can supply an input directory with `-I` and the tool will scan that directory for any `.c` or `.h` files. `bk` analyzes the included files and collects all `typedef struct { field_type field; } StructName` style struct definitions. Each struct can 'derive' functionalities that will be included in the generated code. For instance, if you want your struct to support JSON parsing/dumping you would write:

```c
typedef struct {
    int some_field; tag("SomeField")
    const char* some_other_field;
    /* fields */
} MyStruct derive_json();
```
(See [people.h](./examples/people.h))

By default, `bk` places generated files next to their source files (mirroring the original file structure). These generated files have the following naming schema: `original_file_name.bk.h`.

For further explanation regarding the provided example, check out the [Usage section of User Documentation](./docs/usage.md).

# Documentation
## User Documentation
You can access User Documentation via the [`docs/index.md`](./docs/index.md) file in this repository, this file contains links to other sections of User Documentation.

This documentation contains information about [the general usage of `bookkeeper`](./docs/usage.md), [configuration](./docs/config.md) and [writing extensions for `bookkeeper`](./docs/extensions.md).

## `bookkeeper` Documentation
`bookkeeper` Documentation is generated via [`doxygen`](https://doxygen.nl/). After installing it if you haven't, you can run:
```console
    $ make docs
```
in the root of the repository to generate the documentation. The generated files are placed inside `docs/doxygen` and you can open the `docs/doxygen/html/index.html` file inside your browser to browse the documentation.

This documentation is the general documentation of the internals of `bookkeeper`. It is meant for those who want to modify `bookkeeper`, extension authors or those who are just curious.

# Build Instructions
## Prerequisites
 - [git](https://git-scm.com/)
 - [gnu make](https://www.gnu.org/software/make/)
 - A C compiler (defaults to `clang`, change `CC` in [Makefile](./Makefile) if you prefer `gcc` or another)

## Instructions
First, you will need to clone this repository and `cd` into the repository's root directory:
```console
    $ git clone https://github.com/serd223/bookkeeper.git
    $ cd bookkeeper
```

Then create `build` and `gen` folders:
```console
    $ mkdir build
    $ mkdir gen
```

You can build `bk` by running:
```console
    $ make
    $ ./build/bk
```

## Building Examples
In order to build the `dump_people` example, run: (auto-generates necessary files)
```console
    $ make dump
    $ ./build/dump_people
```

In order to build the `parse_people` example, run: (auto-generates necessary files)
```console
    $ make parse
    $ ./build/parse_people
```
In order to build and test the provided extension example, you can run: (auto-generates necessary files)
```console
    $ make schema_ext
    $ ./build/bk_ext -I ./examples -o ./gen -om dir
```
You can then inspect the generated files inside the `gen` folder to see the generated example schema functions.

Check out [Usage](./docs/usage.md) or the [examples folder](./examples/)!
