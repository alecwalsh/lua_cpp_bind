#include "LuaScript.h"
#include "LuaValue.h"
#include "LuaTable.h"

#include <string>
#include <iostream>


double fn(double i) {
    std::cout << i << std::endl;
    return i;
}

//TODO: add proper unit tests
int main() {
    LuaScript ls{"../test.lua"};

    auto table = LuaTable(ls, "gl_version");
    
    auto r = table["abc"];
    
    std::cout << r << std::endl;
    
    ls.Register("lambda_test", []{std::cout << "Hello from Lua" << std::endl;});
    ls.Register("lambda_test2", [](double i, bool j){std::cout << "Hello from Lua " << i << std::endl;});
    ls.Register("function_test", fn);
    
    
    ls.exec("call_cpp()");
    
//     for(const auto [t1, t2] : table) {
//         std::cout << t1 << " => " << t2 << std::endl;
//     }

    ls.exec("io.write(\"a\")");
    return 0;
}
