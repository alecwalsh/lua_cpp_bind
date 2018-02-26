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

#include "LuaValue.h"
#include "LuaScript.h"

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

