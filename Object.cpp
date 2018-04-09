#include <cstdio>

#include "Object.h"

Object::Object(int i, bool b) : i(i) {
    
}
Object::~Object() {
    printf("destroying object %d\n", i);
}

