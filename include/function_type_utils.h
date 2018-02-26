#pragma once
#include <type_traits>
#include <cstddef>

//Utility classes for finding details of a callable type, such as the return type, argument types, and number of arguments


//Stores a parameter pack
template<typename... Args>
struct pack{pack() = delete;};

//Gets the type of an element in a pack
template<std::size_t, typename...>
struct pack_element;

template<std::size_t I, typename Head, typename... Tail>
struct pack_element<I, pack<Head, Tail...>> {
    using type = typename pack_element<I-1, pack<Tail...>>::type;
};

template<typename Head, typename... Tail>
struct pack_element<0, pack<Head, Tail...>> {
    using type = Head;
};

template<std::size_t I, typename... Ts>
using pack_element_t = typename pack_element<I, Ts...>::type;

template<typename T>
struct pack_size;

template<typename... Args>
struct pack_size<pack<Args...>> {
    static constexpr auto size = sizeof...(Args);
};

template<typename T>
constexpr auto pack_size_v = pack_size<T>::size;;


//Removes the const from a member function pointer's type
//Example: int(ClassType::*)(int, bool) const -> int(ClassType::*)(int, bool)
template<typename T>
struct remove_const_mptr;

template<typename R, typename C, typename... Args>
struct remove_const_mptr<R(C::*)(Args...) const> {
    using type = R(C::*)(Args...);
};

template<typename R, typename G, typename... Args>
struct remove_const_mptr<R(G::*)(Args...)> {
    using type = R(G::*)(Args...);
};

template <typename T>
using remove_const_mptr_t = typename remove_const_mptr<T>::type;


template<typename F>
struct function_type {
private:
    using type_without_const_ref = remove_const_mptr_t<decltype(&std::remove_reference_t<F>::operator())>;
public:
    static constexpr int numargs = function_type<type_without_const_ref>::numargs;
    using return_type = typename function_type<type_without_const_ref>::return_type;
    using args_type = typename function_type<type_without_const_ref>::args_type;
};

template<typename R, typename C, typename... Args>
struct function_type<R(C::*)(Args...)> {
    static constexpr int numargs = sizeof...(Args);
    using return_type = R;
    using args_type = pack<Args...>;
};

template<typename R, typename... Args>
struct function_type<R(Args...)> {
    static constexpr int numargs = sizeof...(Args);
    using return_type = R;
    using args_type = pack<Args...>;
};

//Create a template using a pack as template arguments
//Example:
//using p = pack<int, bool, std::string>;
//apply_pack_t<std::tuple, p> t = {0, false, "abc"};
template<template<typename...> typename, typename>
struct apply_pack;

template<template<typename...> typename T, typename... Args>
struct apply_pack<T, pack<Args...>> {
    using type = T<Args...>;
};

template<template<typename...> typename T, typename... Args>
using apply_pack_t = typename apply_pack<T, Args...>::type;
