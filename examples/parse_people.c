#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "people.h"
#include "cJSON.h"
#define BK_IMPLEMENTATION
#include "people.h.bk.h"

#define JSON_FILE "./examples/manager.json"
int main(void) {
    printf("[INFO] Reading '"JSON_FILE"'...\n");
    struct stat s = {0};
    stat(JSON_FILE, &s);
    size_t file_len = s.st_size / sizeof(char);
    char* input = malloc(file_len); // leaks
    FILE* f = fopen(JSON_FILE, "rb");
    if (!f) return 1;
    size_t input_len = fread(input, sizeof(char), file_len, f) / sizeof(char);
    fclose(f);

    printf("[INFO] Contents of '"JSON_FILE"':\n");
    printf("%.*s\n", (int)input_len, input);

    Manager manager = {0};
    printf("[INFO] Parsing '"JSON_FILE"'...\n");
    if (parse_json_Manager(input, input_len, &manager)) return 1;

    printf("[INFO] Data parsed from '"JSON_FILE"':\n");
    dump_debug_Manager(&manager, stdout);

    printf("\n");
    return 0;
}
