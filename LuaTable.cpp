#include "LuaTable.h"

#include <string>

LuaValue LuaTable::operator[](int i) {
    return table[LuaValue{LUA_TNUMBER, static_cast<double>(i)}];
}
LuaValue LuaTable::operator[](std::string s) {
    return table[LuaValue{LUA_TSTRING, std::string(s)}];
}


LuaTable::LuaTable(LuaScript& ls, const char* t) {
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
    auto t_idx = lua_gettop(L);
    
    //Get the length of the table.  Calls the __len metamethod, assigns the result, and pops it back off the stack
    lua_len(L, -1);
    int length = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    for(int i = 1; i <= length; i++) {
        lua_geti(L, t_idx, i);
        lua_geti(L, -1, 1);
        LuaValue lv1 = LuaValue(ls.L, -1);
        lua_geti(L, -2, 2);
        LuaValue lv2 = LuaValue(ls.L, -1);
        lua_pop(L, 3);
        table.insert({lv1, lv2});
    }
    lua_pop(L, 1);
    LUA_STACK_CHECK_END
}

table_t::iterator LuaTable::begin() noexcept {
    return table.begin();
}

table_t::iterator LuaTable::end() noexcept {
    return table.end();
}
