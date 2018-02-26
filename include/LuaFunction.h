#pragma once

#include <array>
#include <vector>
#include <tuple>
#include <utility>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <any>
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
    LuaFunctionBase* func;
    LuaType return_type;
    std::vector<LuaType> args_types;
};

template<typename F>
struct LuaFunction;

template<typename R, typename... Args>
struct LuaFunction<R(Args...)> : LuaFunctionBase {
private:
    std::array<LuaValue, sizeof...(Args)> args_array;
    
    std::function<R(Args...)> f;
    
    R operator()(Args... args) {
        return f(args...);
    }
    
    template<typename P, std::size_t... I>
    void apply_impl(lua_State* L, std::index_sequence<I...>) {
        auto name = lua_tostring(L, lua_upvalueindex(1));
//         auto type_tuple = *static_cast<std::tuple<LuaType, std::vector<LuaType>>*>(lua_touserdata(L, lua_upvalueindex(3)));
//         auto return_type = std::get<0>(type_tuple);
//         auto args_types = std::get<1>(type_tuple);
        
        std::vector<LuaType> args_types = {static_cast<LuaType>(lua_type(L, I+2))...};
        
        for(int i = 0; i < sizeof...(Args); i++) {
            std::cout << "Type is " << (int)get_lua_types(pack<Args...>{})[i] << std::endl;
            std::cout << "Type2 is " << (int)args_types[i] << std::endl;
        }
        assert(get_lua_types(pack<Args...>{}) == args_types);
//         assert(type_to_lua_type_v<R> == return_type);
        
        if(get_lua_types(pack<Args...>{}) != args_types) {
            std::cout << "Error: type mismatch" << std::endl;
        }
        
        
        //__call always has its table as the first argument
        int expectedargs = sizeof...(Args);
        int numargs = lua_gettop(L) - 1;
        if(expectedargs != numargs) {
            printf("Error: incorrect number of arguments:\n%s called with %d arguments, expected %d\n", name, numargs, expectedargs);
            exit(EXIT_FAILURE);
        }

        //Lua stack starts at 1, and we need to skip the first argument because __call has its table as the first argument
        //So we need to start at 2
        args_array = {{LuaValue{L, ((int)I)+2}...}};
        
//         std::cout << "Size: " << /*sizeof...(Args)*/ args_array.size() << std::endl;

        (*this)(std::any_cast<pack_element_t<I, std::decay_t<P>>>(args_array[I].value)...);
    }
public:
    template<typename F>
    LuaFunction(F&& f) : f(std::forward<F>(f)) {
        
    }
    
    void apply(lua_State* L) {
        apply_impl<pack<Args...>>(L, std::make_index_sequence<sizeof...(Args)>{});
    }
};



// template<typename... Args>
// int apply(std::unordered_map<std::string, LuaFunctionAndTypes> map, std::string key, Args... args) {
//     auto func = map[key];
//     assert(func.args_types == get_lua_types(pack<Args...>{}));
//     
//     return (*static_cast<std::function<int(Args...)>*>(func.fb))(args...);
// }
