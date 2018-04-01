#pragma once
#include <type_traits>
#include <cstddef>

//Utility classes for finding details of a callable type, such as the return type, argument types, and number of arguments


//Stores a parameter pack
template<typename... Args>
struct pack{};

//Gets the type of an element in a pack
template<std::size_t, typename>
struct pack_element;

template<template<typename...> typename T, std::size_t I, typename Head, typename... Tail>
struct pack_element<I, T<Head, Tail...>> {
    using type = typename pack_element<I-1, T<Tail...>>::type;
};

template<template<typename...> typename T, typename Head, typename... Tail>
struct pack_element<0, T<Head, Tail...>> {
    using type = Head;
};

template<std::size_t I, typename T>
using pack_element_t = typename pack_element<I, T>::type;

template<typename T>
struct pack_size;

template<template<typename...> typename T, typename... Args>
struct pack_size<T<Args...>> {
    static constexpr size_t size = sizeof...(Args);
};

template<typename T>
constexpr size_t pack_size_v = pack_size<T>::size;


//Removes the const from a member function pointer's type
//Example: int(ClassType::*)(int, bool) const -> int(ClassType::*)(int, bool)
template<typename T>
struct remove_const_mptr;

template<typename R, typename C, typename... Args>
struct remove_const_mptr<R(C::*)(Args...) const> {
    using type = R(C::*)(Args...);
};

template<typename R, typename C, typename... Args>
struct remove_const_mptr<R(C::*)(Args...)> {
    using type = R(C::*)(Args...);
};

template <typename T>
using remove_const_mptr_t = typename remove_const_mptr<T>::type;

//Gets the return type and argument types of a function type
template<typename F>
struct function_type {
private:
    using type_without_const_ref = remove_const_mptr_t<decltype(&std::remove_reference_t<F>::operator())>;
public:
    static constexpr size_t numargs = function_type<type_without_const_ref>::numargs;
    //R(Args...)
    using type = typename function_type<type_without_const_ref>::type;
    //Return type and types of the arguments
    using return_type = typename function_type<type_without_const_ref>::return_type;
    using args_type = typename function_type<type_without_const_ref>::args_type;
};

template<typename R, typename C, typename... Args>
struct function_type<R(C::*)(Args...)> {
    static constexpr size_t numargs = sizeof...(Args);
    using type = R(Args...);
    using return_type = R;
    using args_type = pack<Args...>;
};

template<typename R, typename... Args>
struct function_type<R(Args...)> {
    static constexpr int numargs = sizeof...(Args);
    using type = R(Args...);
    using return_type = R;
    using args_type = pack<Args...>;
};

template<typename R, typename... Args>
struct function_type<R(*)(Args...)> : function_type<R(Args...)> {};

template<typename R, typename... Args>
struct function_type<R(&)(Args...)> : function_type<R(Args...)> {};


template<typename F>
using function_type_t = typename function_type<F>::type;

template<typename F>
using function_args_type_t = typename function_type<F>::args_type;

template<typename F>
using function_return_type_t = typename function_type<F>::return_type;

template<typename F>
static constexpr size_t function_numargs_v = function_type<F>::numargs;

//Create a template using another template's arguments
//Example:
//using p = pack<int, bool, std::string>;
//apply_pack_t<std::tuple, p> t = {0, false, "abc"};
template<template<typename...> typename, typename>
struct apply_pack;

template<template<typename...> typename T, template<typename...> typename P, typename... Args>
struct apply_pack<T, P<Args...>> {
    using type = T<Args...>;
};

template<template<typename...> typename T, typename P>
using apply_pack_t = typename apply_pack<T, P>::type;
