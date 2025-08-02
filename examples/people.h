#ifndef __PEOPLE_H__
#define __PEOPLE_H__
#include <stdbool.h>
#include "../gen/derives.h"

typedef struct {
    const char* name;
    int age;
    bool married;
} Person derive_all();

typedef struct {
    Person personal_info;
    const char* title;
    double rating;
    bool fired;
    int office_floor;
} Manager derive_json() derive_debug();

#endif // __PEOPLE_H__
