#include "people.h"
#include <stdio.h>
#define BK_FMT(...) offset += fprintf(dst, __VA_ARGS__)
#define DISABLE_PARSE
#include "../gen/bookkeeper.c"

int main() {
    Person manager_personal = {
        .name = "Gabe Newell",
        .age = 62,
        .married = true,
    };
    Manager manager = {
        .personal_info = manager_personal,
        .title = "President",
        .rating = 999999.9999,
        .fired = false,
        .office_floor = 42
    };
    printf("[INFO] JSON Output:\n");
    dump_json_Manager(&manager, stdout);
    printf("\n\n[INFO] Debug Output:\n");
    dump_debug_Manager(&manager, stdout);
    return 0;
}
