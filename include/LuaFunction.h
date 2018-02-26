#pragma once

#include <array>
#include <vector>
#include <tuple>
#include <utility>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <any>
#include <memory>
#include <cassert>

#include "lua.hpp"
#include "LuaTypes.h"
#include "LuaValue.h"

#include "function_type_utils.h"

// template<typename...>
// struct apply_tail;
// 
// template<typename Head, typename... Tail>
// struct apply_tail {
//     
// };

struct LuaFunctionBase {
    virtual void apply(lua_State* L) = 0;
};

//Stores a LuaFunction, its return type, and its argument types
struct LuaFunctionAndTypes {
//     LuaFunctionAndTypes(std::unique_ptr<LuaFunctionBase> func, LuaType return_type, std::vector<LuaType> args_types) : func(std::move(func)), return_type(return_type), args_types(args_types) {}
//     LuaFunctionAndTypes() = default;
    std::unique_ptr<LuaFunctionBase> func;
//     LuaType return_type;
    std::vector<LuaType> args_types;
};

template<typename F>
struct LuaFunction;

template<typename R, typename... Args>
struct LuaFunction<R(Args...)> : LuaFunctionBase {
private:
    std::array<LuaValue, sizeof...(Args)> args_array;
    
    std::function<R(Args...)> f;
    
    template<typename P, std::size_t... I>
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
        std::vector<LuaType> args_types = {static_cast<LuaType>(lua_type(L, I+2))...};
        
//         assert(get_lua_types(pack<Args...>{}) == args_types);
        
        //Compare the argument types this was called with with the expected argument types
        if(get_lua_types<pack<Args...>>() != args_types) {
            std::cerr << "Error: type error in argument" << std::endl;
            exit(EXIT_FAILURE);
        }

        //Lua stack starts at 1, and we need to skip the first argument because __call has its table as the first argument
        //So we need to start at 2
        args_array = {{LuaValue{L, ((int)I)+2}...}};

        f(std::any_cast<pack_element_t<I, std::decay_t<P>>>(args_array[I].value)...);
    }
public:
    template<typename F>
    LuaFunction(F&& f) : f(std::forward<F>(f)) {
        
    }
    
    void apply(lua_State* L) {
        apply_impl<pack<Args...>>(L, std::make_index_sequence<sizeof...(Args)>{});
    }
};
