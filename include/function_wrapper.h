#pragma once

#include <utility>
#include <tuple>

#include "function_type_utils.h"

namespace LuaCppBind {
namespace detail {
    //TODO: warn when conversion cause loss of precision
    template<typename... LuaArgs, typename... Args, size_t... Is>
    std::tuple<LuaArgs...> convert_types_helper(pack<Args...>, std::index_sequence<Is...>, LuaArgs... args) {
        return {static_cast<pack_element_t<Is, Args...>>(args)...};
    }
    
    template<typename... Args, typename... LuaArgs>
    std::tuple<LuaArgs...> convert_types(LuaArgs... args) {
        return convert_types_helper(pack<Args...>{}, std::make_index_sequence<sizeof...(Args)>{}, args...);
    }
    
    template<typename F, typename... Args>
    auto wrap_lua_function_helper(F&& f, pack<Args...>) {
        return [&](corresponding_type_t<Args>... args) {
            std::apply(std::forward<F>(f), convert_types<Args...>(args...));
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
}
