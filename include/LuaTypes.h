#pragma once
#include <utility>
#include <vector>
#include <string>

#include "function_type_utils.h"

namespace LuaCppBind {

//The integer and number types are LUA_INTEGER and LUA_NUMBER from luaconf.h, make sure to use the right types(e.g. Don't use float instead of double)
enum class LuaType {
    Nil = 0,
    Bool = 1,
    LightUserdata = 2,
    Float = 3,
    String = 4,
    Table = 5,
};

template<typename>
struct type_to_lua_type;

template<typename T>
constexpr LuaType type_to_lua_type_v = type_to_lua_type<T>::value;

template<>
struct type_to_lua_type<int> {
    static constexpr LuaType value = LuaType::Float;
};

template<>
struct type_to_lua_type<double> {
    static constexpr LuaType value = LuaType::Float;
};

template<>
struct type_to_lua_type<bool> {
    static constexpr LuaType value = LuaType::Bool;
};

template<>
struct type_to_lua_type<std::string> {
    static constexpr LuaType value = LuaType::String;
};

template<>
struct type_to_lua_type<void> {
    static constexpr LuaType value = LuaType::Nil;
};

namespace detail {
    template<typename P, std::size_t... I>
    const std::array<LuaType, pack_size_v<P>> get_lua_types_impl(std::index_sequence<I...>) {
        return {type_to_lua_type_v<pack_wrapper_element_t<I, P>>...};
    }
}

//Takes a pack of arguments and returns of vector of their Lua types
template<typename... Args>
const std::array<LuaType, sizeof...(Args)> get_lua_types() {
    return detail::get_lua_types_impl<pack<Args...>>(std::make_index_sequence<sizeof...(Args)>{});
}

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
}
