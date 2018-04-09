#include "LuaScript.h"
#include "LuaValue.h"
#include "LuaTable.h"
#include "LuaUserdata.h"

#include "function_wrapper.h"

#include "Object.h"

#include <string>
#include <iostream>

using namespace LuaCppBind;

int fn(int i) {
    std::cout << i << std::endl;
    return i;
}

template<typename T, typename... Args>
auto RegisterConstructor(LuaScript& ls, const std::string& name) {
    lua_pushlightuserdata(ls.L, new constructor<T, Args...>{});
    lua_pushcclosure(ls.L, new_object<T>, 1);
    lua_setglobal(ls.L, name.c_str());
}

//TODO: add proper unit tests
int main() {
    LuaScript ls{"../test.lua"};

    auto table = LuaTable(ls, "gl_version");
    
    auto r = table["abc"];
    
    std::cout << r << std::endl;
    
    auto l = [](int i){std::cout << "Hello from Lua " << i << std::endl;};
    
    ls.Register("lambda_test", []{std::cout << "Hello from Lua" << std::endl;});
    ls.Register("lambda_test2", [](const double& i, bool j){std::cout << "Hello from Lua " << i << std::endl;});
    ls.Register("lambda_test3", l);
    ls.Register("function_test", fn);
    
    
    auto s = "abcd";
    ls.Register("var_test", s, LuaType::String);
    
    
    
//     new_object<Object>(ls, [](lua_State*, int){}, [](lua_State* L){
//         return std::tuple<int>{1};
//     });
    
//     RegisterConstructor<Object, int, bool>(ls, "new_object");
    lua_pushlightuserdata(ls.L, new constructor<Object, int, bool>{});
    lua_pushcclosure(ls.L, new_object<Object>, 1);
    lua_setglobal(ls.L, "new_object");
    
    ls.exec("call_cpp()");
//     for(const auto [t1, t2] : table) {
//         std::cout << t1 << " => " << t2 << std::endl;
//     }
    
    return 0;
}
