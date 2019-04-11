#include "LuaValue.h"
#include "LuaTable.h"

#include <string>
#include <exception>
#include <cstdio>


namespace LuaCppBind {

//TODO: add more types
bool LuaValue::operator==(const LuaValue& rhs) const noexcept {
    if(type == rhs.type) {
        switch(type) {
            case LUA_TNUMBER:
                return any_cast<LUA_NUMBER>(value) == any_cast<LUA_NUMBER>(rhs.value);
                break;
            case LUA_TSTRING:
                return any_cast<std::string>(value) == any_cast<std::string>(rhs.value);
                break;
            case LUA_TBOOLEAN:
                return any_cast<bool>(value) == any_cast<bool>(rhs.value);
                break;
            case LUA_TNIL:
                //Only one possible nil value
                return true;
            default:
                return false;
        }
    }
    return false;
}

//TODO: add more types
LuaValue::LuaValue(lua_State* L, int idx) {
    type = lua_type(L, idx);
    switch(type) {
        case LUA_TNUMBER:
            value = lua_tonumber(L, idx);
            break;
        case LUA_TSTRING:
            value = std::string{lua_tostring(L, idx)};
            break;
        case LUA_TBOOLEAN:
            value = static_cast<bool>(lua_toboolean(L, idx));
            break;
        case LUA_TNIL:
            //I don't think getting here is possible without manually instantiating a LuaValue
            value = nullptr;
            break;
        case LUA_TTABLE:
            //TODO: Recursively call get_lua_table here
            value = table_t{};
            break;
        case LUA_TNONE:
            printf("Invalid index\n");
            exit(EXIT_FAILURE);
        default:
            printf("Lua value has invalid type\n");
            exit(EXIT_FAILURE);
    }
}

//TODO: Add more types
std::string LuaValue::str() const {
    if(!value.has_value() && type != LUA_TNIL) {
        throw std::runtime_error("Error: doesn't contain a value");
    }
    switch(type) {
        case LUA_TNUMBER:
            return std::to_string(any_cast<LUA_NUMBER>(value));
            break;
        case LUA_TBOOLEAN:
            return any_cast<bool>(value) ? "true" : "false";
            break;
        case LUA_TSTRING:
            return any_cast<std::string>(value);
            break;
        case LUA_TNIL:
            return "nil";
            break;
        default:
            return "Other lua type";
            break;
    }
}

std::ostream& operator<<(std::ostream& o, const LuaCppBind::LuaValue& lv) {
    o << lv.str();
    return o;
}
}
