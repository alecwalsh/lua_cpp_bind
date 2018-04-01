#include "LuaScript.h"
#include "LuaValue.h"
#include "LuaTable.h"

#include <string>
#include <iostream>


int fn(double i) {
    std::cout << i << std::endl;
}

int main() {
    LuaScript ls{"../test.lua"};

    auto table = LuaTable(ls, "gl_version");
    
//     auto r = table["abc"];
//     
//     std::cout << r << std::endl;
    
//     ls.Register("test", []{std::cout << "Hello from Lua" << std::endl;});
    ls.Register("test", [](double i, bool j){std::cout << "Hello from Lua " << i << std::endl;});
    ls.Register("test2", fn);
    
//     lua_pushstring(ls.L, "upvalue");
//     lua_pushstring(ls.L, "upvalue2");
//     lua_pushcfunction(ls.L, [](lua_State* L)->int{std::cout << "top: " << lua_gettop(L) << std::endl;return 0;});
//     lua_setglobal(ls.L, "cpp1");
    
    ls.exec("call_cpp()");
    
//     for(const auto [t1, t2] : table) {
//         std::cout << t1 << " => " << t2 << std::endl;
//     }

//     ls.exec("io.write(\"a\")");
    return 0;
}
