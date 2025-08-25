# Extensions

`bookkeeper` supports ['static'](#static-schema-extensions) and ['dynamic'](#dynamic-schema-extensions) extensions. Static extensions directly wrap [`bk.c`](../bk.c) and bake new extensions directly into the executable while dynamic extensions are **zero build plugins** that consist of files in the `.schema` format (basically C but with extra preprocesssing) which are loaded and interpreted during `bookkeeper`'s runtime.

Static extensions *require* you to adapt your build system to them which makes them harder to integrate into already existing projects. Though static extensions have way better performance (performance as in the speed of *generating* code, not the speed of generated code) compared to dynamic ones since they don't need to interpret any files during runtime.

Note that the documentation in this section assumes you know how to use and understand `bookkeeper`, you should at least read the [Usage section](./usage.md) before reading this section.
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

## Dynamic Schema Extensions
You can include your `.schema` files via the [command line](./usage.md#command-line-options) or [configuration files](./config.md), included schema files will be loaded and ready to use just like any other schema.

The code for the schema of `bookkeeper`'s config files, bkconf, is actually written as a dynamic schema extension! After reading this section, you can check out [its schema code inside the examples directory](../examples/bkconf.schema) for a sort of real world example. Try tweaking the schema file and generating the `bkconf` code with `bk -i bk.c -is ./examples/bkconf.schema -o .` while in the root of the repository and look around the generated code inside `bk.c.bk.h`!

Writing `.schema` files is pretty similar to writing static schema extensions, the main difference is that instead of using the internal `bookkeeper` API to automate certain things, you use special 'directives' directly in your code instead.

See the [Directives Cheatsheet](#directives-cheatsheet) for a short list of all directives.

Each dynamic schema file must start with a `name` and then a `derive` field.

(This snippet and most other snippets are taken from the provided [dynamic example](../examples/dynamic.schema), check it out for a working example)
```
name: "dynamic",
derive: "derive_dynamic"
```
Just like static schemas, these fields need to be unique among all schemas.

Then you can start declaring your functions, even though splitting your declarations and implementations isn't required here, it is still recommended.
```c
$dumpguard$
void dump_dynamic_$ty$($ty$* item, $dst$ dst);
$enddumpguard$
```
Here we see our first special directives (surrounded by '$'s). Let's break them down one by one.
 * $dumpguard$: Generates the necessary `#ifndef BK_DISABLE_*` guards for dump functions.
 * $enddumpguard$: Generates the matching `#endif`s for  $dumpguard$.
 * $ty$: This will be replaced with the processed type's name while interpreting this file. So if you were to use this dynamic schema with a type named 'MyStruct', $ty$ would get replaced by 'MyStruct' while generating code for that type.

After you're done with your declarations, you can move on to your implementations.
```c
$implguard$
$dumpguard$
void dump_dynamic_$ty$($ty$* item, $dst$ dst) {
    $offset$ offset = 0; (void)offset;
    $for ty {$
        $fmt$("$tag$: ");
        $if CINT {$
            $fmt$("%d", item->$it$);
        $}$

        $if CSTRING {$
            $fmt$("%s", item->$it$);
        $}$

        $if CBOOL {$
            $fmt$("%s", item->$it$ ? "true" : "false");
        $}$

        $if CEXTERNAL {$
            dump_dynamic_$it.type$(&item->$it$, dst);
        $}$
        $fmt$("\n");

    $}$
}
$enddumpguard$
$endimplguard$
```
Here we see many new special directives, let's break them down one by one. Starting with the simpler ones:
 * $implguard$: Generates the `#ifdef BK_IMPLEMENTATION` (or whatever you set the implementation macro to be) implementation guard.
 * $endimplguard$: Generates the matching `#endif` for $implguard$.
 * $dst$: Replaced with `BK_FMT_DST_t` or whatever you set that macro to be.
 * $offset$: Replaced with `BK_OFFSET_t` or whatever you set that macro to be. Most `BK_FMT`/format functions expect a variable of type $offset$ named `offset` to be in scope. We used the `(void)offset;` trick in the provided example to suppress unused variable warnings in case the format function doesn't use `offset`.
 * $fmt$: Replaced with `BK_FMT` or whatever you set that macro to be. This printf-like macro is how generated code is meant to output into the provided `dst` buffer (formatting macros usually implicilty use the `dst` parameter) so it is recommended that you only use the $fmt$ directive for output and don't use other functions like `printf`.

Now we can move on to for loops:
```c
$for ty {$
/* ... */
$}$
```
The code inside of this loop will be interpreted for *each field of the type*. Let's imagine the following scenerio:

We have this dynamic schema:
```c
/* ... */
// Inside the .schema file
void dump_dynamic_$ty$($ty$* item, $dst$ dst) {
  $for ty {$
    $fmt$("Hello, World!\n");
  $}$
}
/* ... */
```

And we have this type that we will generate code for:
```c
typedef struct {
  int a;
  bool b;
  char* c;
} MyStruct derive_all();
```

The code generated from the dynamic schema for the `MyStruct` type would look like this: (the interpreter trims unnecessary whitespace so it wouldn't actually be as readable)
```c
/* ... */
// Inside the generated .bk.h file
void dump_dynamic_MyStruct(MyStruct* item, BK_FMT_DST_t dst) {
  BK_FMT("Hello, World!\n");
  BK_FMT("Hello, World!\n");
  BK_FMT("Hello, World!\n");
}
/* ... */
```
As you can see, we ended up with three "Hello, World!"s since `MyStruct` has three fields.

For loops bring some extra directives into scope that can **only be used in for loops**. These are:
 * $it$: Replaced with the name of the field in the current iteration (short for iterator). Can be used like item->$it$ to access fields.
 * $it.type$: If $it$ is an 'external field', which just means that $it$ is not of a primitive type, you can use $it.type$ to get its type. This is how `bookkeeper` is able to seamlessly support nested types. This directive is commonly used as `dump_dynamic_$it.type$(&item->$it$, dst)` to call the correct generated function for this field.
 * $tag$: Replaced with the tag of the field in the current iteration. Is equal to $it$ if the field doesn't have a `tag` attribute in the type's definition.

If statements are also directives that can only be used inside for loops. They are usually used to check the type of the current field.
```c
/* ... */
// Inside the .schema file
void dump_dynamic_$ty$($ty$* item, $dst$ dst) {
  $for ty {$
    $if CINT {$
      $fmt$("%d", item->$it$);
    $}$
  $}$
}
/* ... */
```
This snippet generates a function that only prints the integer fields of the struct.

If we were to use it on `MyStruct` which we had defined previously, the generated code would look like this:
```c
/* ... */
// Inside the generated .bk.h file
void dump_dynamic_MyStruct(MyStruct* item, BK_FMT_DST_t dst) {
  BK_FMT("%d", item->a);
}
/* ... */
```

We can further extend this example to support more types like:
```c
/* ... */
// Inside the .schema file
void dump_dynamic_$ty$($ty$* item, $dst$ dst) {
  $for ty {$
    $if CINT {$
      $fmt$("%d", item->$it$);
    $}$
    $if CSTRING {$
      $fmt$("%s", item->$it$);
    $}$
    $if CBOOL {$
      $fmt$("%s", item->$it$ ? "true" : "false");
    $}$
  $}$
}
/* ... */
```

And even make the output a bit prettier:
```c
/* ... */
// Inside the .schema file
void dump_dynamic_$ty$($ty$* item, $dst$ dst) {
  $for ty {$
    $fmt$("$tag$: ");
    $if CINT {$
      $fmt$("%d", item->$it$);
    $}$
    $if CSTRING {$
      $fmt$("%s", item->$it$);
    $}$
    $if CBOOL {$
      $fmt$("%s", item->$it$ ? "true" : "false");
    $}$
    $fmt$("\n");
  $}$
}
/* ... */
```
Here is a list of all supported types inside if statements:
 * CINT
 * CUINT
 * CLONG
 * CULONG
 * CCHAR
 * CFLOAT
 * CBOOL
 * CSTRING
 * CEXTERNAL

Let's see what the generated code of this schema would look like for `MyStruct`:
```c
/* ... */
// Inside the generated .bk.h file
void dump_dynamic_MyStruct(MyStruct* item, BK_FMT_DST_t dst) {
  BK_FMT("a: ");
  BK_FMT("%d", item->a);
  BK_FMT("\n");
  BK_FMT("b: ");
  BK_FMT("%s", item->b ? "true" : "false");
  BK_FMT("\n");
  BK_FMT("c: ");
  BK_FMT("%s", item->c);
  BK_FMT("\n");
}
/* ... */
```

A few things to note here:
 - The code *outside* of the if statements was interpreted as-is for every single field.
 - The code *inside* of the if statements was interpreted only if the field's type matched the type in the condition.
 - If we wanted to use $it.type$, we could only do so inside a `$if CEXTERNAL {$ ... $}$` block.

Another use for if statements is checking the current field's index. For example:
```c
/* ... */
// Inside the .schema file
$for ty {$
  $if index == 0 {$
    $fmt$("This is only going to be generated for the first field!\n");
  $}$
$}$
/* ... */
```
In this case, the $fmt$ statement inside of the if statement would only get interpreted for the *first field*. This feature is mainly used to differentiate between the starting `if` and `else if`s in a generated if-else if chain. The supported binary operations for this style of if statements are:
 * ==
 * !=

### Directives Cheatsheet
  * $implguard$: The `#ifdef BK_IMPLEMENTATION` (or whatever you set the implementation macro to be) implementation guard.
  * $endimplguard$: The matching `#endif` for $implguard$.
  * $dumpguard$: The necessary `#ifndef BK_DISABLE_*` guards for dump functions.
  * $enddumpguard$: The matching `#endif`s for  $dumpguard$.
  * $ty$: The processed type's name.
  * $dst$: `BK_FMT_DST_t` or whatever you set that macro to be.
  * $offset$: `BK_OFFSET_t` or whatever you set that macro to be. Don't forget to put `$offset$ offset = 0; (void)offset;` at the start of your dump functions to avoid errors and unnecessary warnings.
  * $fmt$: `BK_FMT` or whatever you set that printf-like macro to be.
  * $for ty {$ ... $}$: Iterates over fields.
    - $it$: Current field name.
    - $tag$: Current field tag.
    - $it.type$: Current field's type's name. Can only be used inside `$if CEXTERNAL {$ ... $}$` blocks.
    - $if <type> {$ ... $}$: Only interpreted if the current field is of type `<type>`. Valid values for `<type>` are:
       - CINT
       - CUINT
       - CLONG
       - CULONG
       - CCHAR
       - CFLOAT
       - CBOOL
       - CSTRING
       - CEXTERNAL
    - $if index == <integer> {$ ... $}$: Only interpreted if the current field's index is equal to the provided number.
    - $if index != <integer> {$ ... $}$: Only interpreted if the current field's index is not equal to the provided number.
