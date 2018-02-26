#pragma once
#include <utility>
#include <type_traits>

#include "function_type_utils.h"

// #define LUA_TNONE		(-1)
// 
// #define LUA_TNIL		0
// #define LUA_TBOOLEAN		1
// #define LUA_TLIGHTUSERDATA	2
// #define LUA_TNUMBER		3
// #define LUA_TSTRING		4
// #define LUA_TTABLE		5
// #define LUA_TFUNCTION		6
// #define LUA_TUSERDATA		7
// #define LUA_TTHREAD		8

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
    std::vector<LuaType> get_lua_types_impl(std::index_sequence<I...>) {
        return {type_to_lua_type_v<pack_element_t<I, P>>...};
    }
}

//Takes a pack of arguments and returns of vector of their Lua types
template<typename P>
std::vector<LuaType> get_lua_types() {
    return detail::get_lua_types_impl<P>(std::make_index_sequence<pack_size_v<P>>{});
}
