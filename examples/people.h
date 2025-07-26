#ifndef __PEOPLE_H__
#define __PEOPLE_H__
#include <stdbool.h>
#include "../bookkeeper.h"

typedef struct {
    const char* name;
    int age;
    bool married;
} Person derive_json();

typedef struct {
    Person personal_info;
    const char* title;
    double rating;
    bool fired;
    int office_floor;
} Manager derive_json();

#endif // __PEOPLE_H__
