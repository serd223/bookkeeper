#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "people.h"
#define BK_FMT(...) offset += fprintf(dst, __VA_ARGS__)
#include "../thirdparty/cJSON.h"
#include "../gen/bookkeeper.c"

typedef struct {
    char* items;
    size_t len;
    size_t cap;
} String;

// i/o util
size_t read_entire_file(const char* file_name, String* out);

#define JSON_FILE "./examples/manager.json"

int main() {
    String input = {0};
    printf("[INFO] Reading '"JSON_FILE"'...\n");
    if (!read_entire_file(JSON_FILE, &input)) return 1;
    printf("[INFO] Contents of '"JSON_FILE"':\n");
    printf("%.*s\n", (int)input.len, input.items);
    Manager manager = {0};
    printf("[INFO] Parsing '"JSON_FILE"'...\n");
    if (!parse_json_Manager(input.items, input.len, &manager)) return 1;
    printf("[INFO] Data parsed from '"JSON_FILE"':\n");
    dump_json_Manager(&manager, stdout);
    printf("\n");
    return 0;
}

size_t read_entire_file(const char* file_name, String* out) {
    struct stat s = {0};
    stat(file_name, &s);
    size_t f_len = s.st_size / sizeof(char);
    if (f_len >= (out->cap - out->len)) {
        out->cap += f_len; // allocate extra space beacuse why not ram is cheap
        if (out->items) {
            out->items = realloc(out->items, out->cap);
        } else {
            out->items = malloc(out->cap);
        }
    }
    FILE* f = fopen(file_name, "rb");
    if (!f) return 0;
    size_t read = fread(out->items + out->len, sizeof(char), f_len, f) / sizeof(char);
    out->len += read;
    return read;
}
