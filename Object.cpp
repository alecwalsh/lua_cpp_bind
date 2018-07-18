#include <cstdio>

#include "Object.h"

Object::Object(int i, bool b) : i(i) {
    
}
Object::~Object() {
    printf("destroying object with i = %d\n", i);
}

void Object::printi() {
    printf("calling method - %p\n", this);
    printf("i = %d\n", i);
}

