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

#include <lua.hpp>
#include <string>

struct LuaValue {
    int type;
    any value;
    
    LuaValue(int type, any value) : type(type), value(value) {}
    LuaValue() : type(LUA_TNONE), value(nullptr) {}
    LuaValue(lua_State* L, int idx);
    
    //TODO: add more types
    bool operator==(const LuaValue& rhs) const noexcept;
    
    std::string str() const;
};
