#pragma once
#include "LuaScript.h"
#include "LuaValue.h"
using table_t = std::unordered_map<LuaValue, LuaValue>;

struct LuaTable {
    LuaTable(LuaScript& ls, const char* t);
    table_t table;
    table_t::iterator begin() noexcept;
    table_t::iterator end() noexcept;
    LuaValue operator[](int i);
    LuaValue operator[](std::string s);
};

//TODO: add more types
LuaValue get_lua_value(LuaScript& ls, int idx);
