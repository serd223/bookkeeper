#include <stdio.h>

#define BK_IMPLEMENTATION
#define BK_DISABLE_json_PARSE

#include "people.h" // Example file
#include "people.h.bk.h" // Generated file

typedef struct {
    Person friend_info;
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
