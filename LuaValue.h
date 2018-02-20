#pragma once
#if __cplusplus >= 201703L
#include <any>
using std::any;
using std::any_cast;
#else
#include <boost/any.hpp>
using boost::any;
using boost::any_cast;
#endif

#include "LuaScript.h"

struct LuaValue {
    int type;
    any value;
    
    //TODO: add more types
    bool operator==(const LuaValue& rhs) const noexcept;
};

namespace std {
    template<> struct hash<LuaValue> {
        using argument_type = LuaValue;
        using result_type = std::size_t;
        
        //TODO: implement this
        std::size_t operator()(const LuaValue& lv) const noexcept {
            switch(lv.type) {
                case LUA_TNUMBER:
                    return std::hash<LUA_NUMBER>()(any_cast<LUA_NUMBER>(lv.value));
                case LUA_TBOOLEAN:
                    return std::hash<bool>()(any_cast<bool>(lv.value));
                case LUA_TSTRING:
                    return std::hash<std::string>()(any_cast<std::string>(lv.value));
                default:
                    return 0;
            }
        }
    };
}

using table_t = std::unordered_map<LuaValue, LuaValue>;

//TODO: add more types
LuaValue get_lua_value(LuaScript& ls, int idx);

table_t get_lua_table(LuaScript& ls, const char* t);
