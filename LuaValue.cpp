#include "LuaValue.h"

#include <string>
#include <cstdio>

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
//TODO: move to constructor
LuaValue get_lua_value(LuaScript& ls, int idx) {
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
            //I don't think getting here is possible without manually instantiating a LuaValue
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

table_t get_lua_table(LuaScript& ls, const char* t) {
    table_t table;
    auto L = ls.L;
    LUA_STACK_CHECK_START
    
    //Push function onto stack
    ls.exec(R"(
return function(t)
    local out = {}
    local outidx = 1
    local count = 0
    for k,v in pairs(t) do
        out[outidx] = {k,v}
        outidx = outidx + 1
        count = count + 1
    end
    return setmetatable(out, {__len = function() return count end})
end
)");
    
    //Push argument onto stack
    lua_getglobal(L, t);
    if(!lua_istable(L, -1)) {
        //TODO: handle this without exiting
        printf("Not a table\n");
        exit(1);
    }
    //Calls the function.  Now the out table is on the stack
    lua_pcall(L, 1, 1, 1);
//     lua_setglobal(L, "aaaa");
//     
//     ls.exec(R"(
//         for k,v in pairs(aaaa) do
//             io.write(k, "\n")
//         end
//        )");
//     exit(1);
    auto t_idx = lua_gettop(L);
    
    //Get the length of the table.  Calls the __len metamethod, assigns the result, and pops it back off the stack
    lua_len(L, -1);
    int length = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    for(int i = 1; i <= length; i++) {
        lua_geti(L, t_idx, i);
        lua_geti(L, -1, 1);
        LuaValue lv1 = get_lua_value(ls, -1);
        lua_geti(L, -2, 2);
        LuaValue lv2 = get_lua_value(ls, -1);
        lua_pop(L, 3);
        table.insert({lv1, lv2});
    }
    lua_pop(L, 1);
    LUA_STACK_CHECK_END
    return table;
}
