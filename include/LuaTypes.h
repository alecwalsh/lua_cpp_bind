#pragma once
#include <utility>
#include <vector>
#include <string>

#include "function_type_utils.h"

//The integer and number types are LUA_INTEGER and LUA_NUMBER from luaconf.h, make sure to use the right types(e.g. Don't use double instead of float)
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
constexpr auto type_to_lua_type_v = type_to_lua_type<T>::value;

template<>
struct type_to_lua_type<int> {
    static constexpr auto value = LuaType::Float;
};

template<>
struct type_to_lua_type<double> {
    static constexpr auto value = LuaType::Float;
};

template<>
struct type_to_lua_type<bool> {
    static constexpr auto value = LuaType::Bool;
};

template<>
struct type_to_lua_type<std::string> {
    static constexpr auto value = LuaType::String;
};

template<>
struct type_to_lua_type<void> {
    static constexpr auto value = LuaType::Nil;
};

namespace detail {
    template<typename P, std::size_t... I>
    std::array<LuaType, pack_size_v<P>> get_lua_types_impl(std::index_sequence<I...>) {
        return {type_to_lua_type_v<pack_element_t<I, P>>...};
    }
}

//Takes a pack of arguments and returns of vector of their Lua types
template<typename P>
std::array<LuaType, pack_size_v<P>> get_lua_types() {
    return detail::get_lua_types_impl<P>(std::make_index_sequence<pack_size_v<P>>{});
}
