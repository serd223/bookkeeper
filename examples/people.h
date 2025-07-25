#ifndef __PEOPLE_H__
#define __PEOPLE_H__
#include <stdbool.h>

typedef struct {
    const char* name;
    double weight;
    int age;
    bool married;
} Person;

typedef struct {
    Person personal_info;
    const char* title;
    double rating;
    bool fired;
    int office_floor;
} Manager;

#endif // __PEOPLE_H__
