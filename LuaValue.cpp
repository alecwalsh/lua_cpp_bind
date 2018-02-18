#include "LuaValue.h"

#include <string>
#include <cstdio>

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
LuaValue get_lua_value(const LuaScript& ls, int idx) {
    auto L = ls.L;
    auto type = lua_type(L, idx);
    any value;
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
            value = nullptr;
            break;
        case LUA_TTABLE:
            //TODO: Recursively call get_lua_table here
            value = table_t{};
            break;
        case LUA_TNONE:
            printf("Invalid index\n");
            exit(1);
        default:
            printf("Lua value has invalid type\n");
            exit(1);
    }
    return {type, value};
}

table_t get_lua_table(const LuaScript& ls, const char* t) {
    auto L = ls.L;
    LUA_STACK_CHECK_START
    table_t table;
    
    lua_getglobal(L, t);
    if(!lua_istable(L, -1)) {
        printf("Not a table\n");
        exit(1);
    }
    auto t_idx = lua_gettop(L);
    lua_pushnil(L);  /* first key */
    
    while (lua_next(L, t_idx) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        auto lv1 = get_lua_value(ls, -1);
        auto lv2 = get_lua_value(ls, -2);
        table[lv2] = lv1;
//         table[get_lua_value(ls, -2)] = get_lua_value(ls, -1);
        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    LUA_STACK_CHECK_END
    return table;
}
