#pragma once
#if __has_include(<any>) && __cplusplus >= 201703L
#include <any>
using std::any;
using std::any_cast;
#elif __has_include(<experimental/any>)
#include <experimental/any>
using std::experimental::any_cast;

struct my_any : public std::experimental::any {
    using any::any;
    bool has_value() const noexcept {
        return !empty();
    }
};
using any = my_any;
#else
#error Requires std::any
#endif

#include <lua.hpp>

#include <ostream>
#include <unordered_map>
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
    
    template<typename T>
    T get();
};

template<typename T>
T LuaValue::get() {
    return any_cast<T>(value);
}


std::ostream& operator<<(std::ostream& o, const LuaValue& lv);

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
