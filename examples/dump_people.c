#include "people.h"
#include "stdio.h"
#define BK_FMT(...) offset += fprintf(dst, __VA_ARGS__)
#include "../bookkeeper.c"

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
    dump_json_Manager(&manager, stdout);
    printf("\n");
    return 0;
}
