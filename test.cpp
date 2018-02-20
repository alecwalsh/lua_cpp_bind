#include "LuaScript.h"
#include "LuaValue.h"

#include <string>
#include <sstream>
#include <iostream>

std::string lua_value_to_string(const LuaValue& lv) {
    std::stringstream result;
    if(!lv.value.has_value()) {
        return "Error: doesn't contain a value";
    }
    switch(lv.type) {
        case LUA_TNUMBER:
            result << "LUA_TNUMBER = " << any_cast<LUA_NUMBER>(lv.value);
            return result.str();
            break;
        case LUA_TBOOLEAN:
            result << "LUA_TBOOLEAN = " << any_cast<bool>(lv.value);
            return result.str();
            break;
        case LUA_TSTRING:
            result << "LUA_TSTRING = " << any_cast<std::string>(lv.value);
            return result.str();
            break;
        case LUA_TNIL:
            return "LUA_TNIL";
            break;
        default:
            return "Other lua type";
            break;
    }
}

int main() {
    LuaScript ls{"../test.lua"};
    
    table_t table = get_lua_table(ls, "gl_version");
    
    for(auto t : table) {
        std::cout << lua_value_to_string(t.first) << " => " << lua_value_to_string(t.second) << std::endl;
    }

//     ls.exec("io.write(\"a\")");
    return 0;
}
