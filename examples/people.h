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
    Person personal_info; tag("PersonalInfo")
    const char* title; tag("Title")
    double rating; tag("Rating")
    bool fired; tag("Fired")
    int office_floor; tag("OfficeFloor")
} Manager derive_json() derive_debug();

#endif // __PEOPLE_H__
