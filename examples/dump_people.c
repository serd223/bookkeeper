#include "people.h"
#include <stdio.h>

// You don't have to put all your definitions in a seperate header file, you can still use
// `bookkeeper` inside of your regular C files. You just need to put the `*.bk.h` include
// after your struct definitions. (since those function declarations and implementations need the type)
typedef struct {
    Manager inner;
    unsigned long salary;
} ManagerWithSalary derive_all();

// This is how you could redefine BK_FMT (and also make use of `offset`)
// #define BK_FMT_DST_t char*
// #define BK_FMT(...) offset += sprintf(dst + offset, __VA_ARGS__)

// This is the default BK_FMT implementation with `fprintf`
#define BK_FMT_DST_t FILE*
#define BK_FMT(...) offset += fprintf(dst, __VA_ARGS__)
#define DISABLE_PARSE
#define BK_IMPLEMENTATION
#include "../gen/people.h.bk.h"
#include "../gen/dump_people.c.bk.h"

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
    ManagerWithSalary manager_with_salary = {
        .inner = manager,
        .salary = 123456789
    };
    printf("[INFO] JSON Output:\n");
    dump_json_ManagerWithSalary(&manager_with_salary, stdout);
    printf("\n\n[INFO] Debug Output:\n");
    dump_debug_ManagerWithSalary(&manager_with_salary, stdout);
    return 0;
}
