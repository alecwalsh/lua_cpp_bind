#pragma once

#include <array>
#include <tuple>
#include <utility>
#include <functional>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <any>
#include <memory>
#include <cassert>

#include "LuaTypes.h"
#include "LuaValue.h"

#include "function_type_utils.h"

struct LuaArgumentTypeError : public std::runtime_error {
    using runtime_error::runtime_error;
};

struct LuaFunctionBase {
    virtual void apply(lua_State* L) = 0;
};

template<typename F>
struct LuaFunction;

template<typename R, typename... Args>
struct LuaFunction<R(Args...)> : LuaFunctionBase {
private:
    std::function<R(Args...)> f;
    std::array<LuaType, sizeof...(Args)> args_types;
    
    template<std::size_t... I>
    void apply_impl(lua_State* L, std::index_sequence<I...>) {
        auto name = lua_tostring(L, lua_upvalueindex(1));
        
        //__call always has its table as the first argument
        int expectedargs = sizeof...(Args);
        int numargs = lua_gettop(L) - 1;
        if(expectedargs != numargs) {
            char buf[64];
            snprintf(buf, 64, "%s called with %d arguments, expected %d", name, numargs, expectedargs);
            throw LuaArgumentTypeError{buf};
        }
        
        //The argument types this was called with
        std::array<LuaType, sizeof...(Args)> args_types = {static_cast<LuaType>(lua_type(L, I+2))...};
        
        //Compare the argument types this was called with with the expected argument types
        if(get_lua_types<pack<Args...>>() != args_types) {
            char buf[64];
            snprintf(buf, 64, "Type error in call to Lua function %s", name);
            throw LuaArgumentTypeError{buf};
        }
        
        //Lua stack starts at 1, and we need to skip the first argument because __call has its table as the first argument
        //So we need to start at 2
        f(LuaValue{L, I+2}.get<pack_element_t<I, pack<Args...>>>()...);
    }
public:
    template<typename F>
    LuaFunction(F&& f) : f(std::forward<F>(f)), args_types(get_lua_types<pack<Args...>>()) {}
    
    void apply(lua_State* L) {
        apply_impl(L, std::make_index_sequence<sizeof...(Args)>{});
    }
};
