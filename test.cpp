#include "LuaScript.h"
#include "LuaValue.h"
#include "LuaTable.h"

#include <string>
#include <iostream>

int main() {
    LuaScript ls{"../test.lua"};
    
    auto table = LuaTable(ls, "gl_version");
    
    auto r = table["abc"];
    
    std::cout << r << std::endl;
    
    for(const auto [t1, t2] : table) {
        std::cout << t1 << " => " << t2 << std::endl;
    }

//     ls.exec("io.write(\"a\")");
    return 0;
}
