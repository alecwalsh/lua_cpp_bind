﻿#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include <utility>
#include <tuple>

#include <cassert>

#include <lua.hpp>

#include "LuaTypes.h"
#include "LuaFunction.h"

#include "function_type_utils.h"

//Use these at the beginning and end of function to make sure you left Lua's stack the way you found it
#ifdef DEBUG
#define LUA_STACK_CHECK_START int starttop = lua_gettop(L);
#define LUA_STACK_CHECK_END assert(starttop == lua_gettop(L));
#else
#define LUA_STACK_CHECK_START
#define LUA_STACK_CHECK_END
#endif

//Call a C++ method
int call_cpp(lua_State* L);

//Set a C++ value
int set_cpp(lua_State* L);

//Get a C++ value
int get_cpp(lua_State* L);

//Stores a LuaFunction, its return type, and its argument types
struct LuaFunctionAndTypes;

class LuaScript {
protected:
public:
    using Type = LuaType;
    lua_State* L;
    //Pointer to a member function
    template<typename R, typename T> using mptr_t = R(T::*);
    
    //Register a normal variable
    template<typename T>
    void Register(std::string name, T ptr, Type type);
    
    //TODO: Can't change the object the member function is called on
    //Register a member function
    template<typename R, typename T, typename... Args>
    void Register(std::string name, mptr_t<R,T> mptr, T* obj, Args&&... args);
    
    //Register any Callable
    template<typename F>
    void Register(std::string name, F&& f);
    
    void exec(std::string code);
    
    LuaScript();
    LuaScript(std::string fileName);
    virtual ~LuaScript();
private:
    void SetupBinding();
    std::unordered_map<std::string, std::pair<void*, Type>> propertyMap;
    std::unordered_map<std::string, LuaFunctionAndTypes*> methodMap;
};

template<typename T>
void LuaScript::Register(std::string name, T ptr, Type type) {
    propertyMap.insert({name, {ptr, type}});
}

// template<typename R, typename T, typename... Args>
// void LuaScript::Register(std::string name, mptr_t<R,T> mptr, T* obj, Args&&... args) {
//     LUA_STACK_CHECK_START
//     //Creates a lambda that calls mptr on obj with args
//     //Args is passed by reference, so you can change them after calling Register
//     methodMap.insert({name, [=, &args...](){return (obj->*mptr)(std::forward<Args>(args)...);}});
//     
//     lua_pushstring(L, name.c_str());
//     lua_pushlightuserdata(L, &methodMap);
//     lua_pushcclosure(L, call_cpp, 2);
//     lua_setglobal(L, name.c_str());
//     LUA_STACK_CHECK_END
// }

template<typename R, typename... Args>
LuaFunctionAndTypes* createLuaFunctionAndTypes(std::function<R(Args...)> f) {
    using args_type = typename function_type<decltype(f)>::args_type;
    using return_type = typename function_type<decltype(f)>::return_type;
    
    return new LuaFunctionAndTypes{new LuaFunction<R(Args...)>{f}, type_to_lua_type_v<return_type>, get_lua_types(args_type{})};
}

template<typename F>
void LuaScript::Register(std::string name, F&& f) {
    LUA_STACK_CHECK_START
    methodMap.insert({name, createLuaFunctionAndTypes(std::function(f))});
    
//     using args_type = typename function_type<decltype(f)>::args_type;
//     using return_type = typename function_type<decltype(f)>::return_type;
//     //TODO: replace with full userdata, use __gc metamethod to free
//     auto function_type_tuple = new std::tuple<LuaScript::Type, std::vector<LuaScript::Type>>{type_to_lua_type_v<return_type>, get_lua_types(args_type{})};
    
    //Create a table
    lua_createtable(L, 0, 0);
    auto table_idx = lua_gettop(L);
    
    //Create the metatable
    lua_createtable(L, 0, 0);
    auto metatable_idx = lua_gettop(L);
    lua_pushstring(L, "__call");
    lua_pushstring(L, name.c_str());
    lua_pushlightuserdata(L, &methodMap);
//     lua_pushlightuserdata(L, function_type_tuple);
    lua_pushcclosure(L, call_cpp, 2);
    lua_settable(L, metatable_idx);

    lua_setmetatable(L, table_idx);
    
    lua_pushstring(L, "numargs");
    lua_pushinteger(L, function_type<F>::numargs);
    lua_settable(L, table_idx);
    
//     lua_pushstring(L, "type");
//     lua_pushlightuserdata(L, function_type_tuple);
//     lua_settable(L, table_idx);
    
    lua_setglobal(L, name.c_str());
    LUA_STACK_CHECK_END
}
