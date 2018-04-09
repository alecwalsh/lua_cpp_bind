#pragma once

#include <utility>
#include <tuple>

#include "function_type_utils.h"

namespace detail {
    template<typename T>
    struct corresponding_type {
        using type = T;
    };
    
    template<typename T>
    struct corresponding_type<const T&> {
        using type = T;
    };

    template<>
    struct corresponding_type<int> {
        using type = double;
    };
    
    template<>
    struct corresponding_type<float> {
        using type = double;
    };

    template<typename T>
    using corresponding_type_t = typename corresponding_type<T>::type;

    //TODO: warn when conversion cause loss of precision
    template<typename LuaArg, typename Arg>
    auto convert_type(LuaArg t) {
        return static_cast<Arg>(t);
    }
    template<typename... LuaArgs, typename... Args, size_t... Is>
    std::tuple<LuaArgs...> convert_types_helper(pack<Args...>, std::index_sequence<Is...>, LuaArgs... args) {
        return {convert_type<pack_element_t<Is, pack<LuaArgs...>>, pack_element_t<Is, pack<Args...>>>(args)...};
    }
    
    template<typename... LuaArgs, typename... Args>
    std::tuple<LuaArgs...> convert_types(pack<Args...>, LuaArgs... args) {
        return convert_types_helper(pack<Args...>{}, std::make_index_sequence<sizeof...(Args)>{}, args...);
    }
    
    template<typename F, typename... Args>
    auto wrap_lua_function_helper(F&& f, pack<Args...>) {
        //Directly capturing f causes a segfault when using gcc, use std::function as a workaround
        //I think it's this bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70515
        std::function f2{f};
        return [&, f2](corresponding_type_t<Args>... args) {
            std::apply(f2, convert_types(pack<Args...>{}, args...));
        };
    }
}


//lua_tonumber returns a double, and any_cast<int> doesn't accept doubles
//This wraps a function in another function that converts the types from Lua into the types the C++ function uses
//So a function with an int as an argument is wrapped in a function with a double as an argument, which casts the double to an int and calls the original function
template<typename F>
auto wrap_lua_function(F&& f) {
    return detail::wrap_lua_function_helper(std::forward<F>(f), function_args_type_t<F>{});
}
