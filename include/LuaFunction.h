#pragma once

#include <array>
#include <tuple>
#include <utility>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <any>
#include <memory>
#include <cassert>

#include "LuaTypes.h"
#include "LuaValue.h"

#include "function_type_utils.h"

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
            printf("Error: incorrect number of arguments:\n%s called with %d arguments, expected %d\n", name, numargs, expectedargs);
            exit(EXIT_FAILURE);
        }
        
        //The argument types this was called with
        std::array<LuaType, sizeof...(Args)> args_types = {static_cast<LuaType>(lua_type(L, I+2))...};
        
        //Compare the argument types this was called with with the expected argument types
        if(get_lua_types<pack<Args...>>() != args_types) {
            std::cerr << "Error: type error in argument" << std::endl;
            exit(EXIT_FAILURE);
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
