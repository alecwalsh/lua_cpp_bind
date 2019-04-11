#pragma once

#include <array>
#include <tuple>
#include <utility>
#include <functional>
#include <string>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <memory>
#include <cassert>

#include "LuaTypes.h"
#include "LuaValue.h"
#include "LuaFunction.h"

#include "function_type_utils.h"

namespace LuaCppBind {

class LuaMethodBase {
public:
    virtual void apply(lua_State* L) = 0;
    virtual ~LuaMethodBase() {}
};

//TODO: this should share most of the logic with LuaFunction
template<typename F>
class LuaMethod;

template<typename T, typename R, typename... Args>
class LuaMethod<R(T&, Args...)> : public LuaMethodBase {
    const std::function<R(T&, Args...)> f;
    const std::array<LuaType, sizeof...(Args)> args_types;
    
    template<std::size_t... Is>
    void apply_impl(lua_State* L, std::index_sequence<Is...>) {
        auto name = lua_tostring(L, lua_upvalueindex(1));
        
        T& object = *static_cast<T*>(lua_touserdata(L, lua_upvalueindex(3)));
        
        int expectedargs = sizeof...(Args);
        //__call always has its table as the first argument, so subtract 1, and methods have self as the first arg, so subtract another 1
        int numargs = lua_gettop(L) - 2;
        
        if(expectedargs != numargs) {
            std::string s = name;
            s += " called with ";
            s += std::to_string(numargs);
            s += " arguments, expected ";
            s += std::to_string(expectedargs);
            
            throw LuaArgumentCountError{s};
        }
        
        //The argument types this was called with
        std::array<LuaType, sizeof...(Args)> args_types = {static_cast<LuaType>(lua_type(L, Is+2))...};
        
        //Compare the argument types this was called with with the expected argument types
        if(get_lua_types<Args...>() != args_types) {
            std::string s = "Type error in call to Lua function ";
            s += name;
            
            throw LuaArgumentTypeError{s};
        }
        
        //Lua stack starts at 1, and we need to skip the first argument because __call has its table as the first argument
        //So we need to start at 2
        f(object, LuaValue{L, Is+2}.get<pack_element_t<Is, pack<Args...>>>()...);
    }
public:
    template<typename F>
    LuaMethod(F&& f) : f(std::forward<F>(f)), args_types(get_lua_types<Args...>()) {}
    
    void apply(lua_State* L) {
        apply_impl(L, std::index_sequence_for<Args...>{});
    }
};
}
