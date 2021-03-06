﻿#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include <utility>
#include <tuple>
#include <memory>
#include <cassert>

#include "LuaTypes.h"
#include "LuaFunction.h"
#include "LuaValue.h"

#include "function_type_utils.h"
#include "function_wrapper.h"

//Use these at the beginning and end of function to make sure you left Lua's stack the way you found it
#ifdef DEBUG
#define LUA_STACK_CHECK_START int starttop = lua_gettop(L);
#define LUA_STACK_CHECK_END assert(starttop == lua_gettop(L));
#else
#define LUA_STACK_CHECK_START
#define LUA_STACK_CHECK_END
#endif

namespace LuaCppBind {

class LuaScript {
    //Call a C++ method
    static int call_cpp(lua_State* L);
    //Set a C++ value
    static int set_cpp(lua_State* L);
    //Get a C++ value
    static int get_cpp(lua_State* L);
public:
    using Type = LuaType;
    lua_State* L;
    //Pointer to a member function
    
    //Register a normal variable
    template<typename T>
    void Register(const std::string& name, T& val, Type type);
    
    //Register any Callable
    //TODO: Support returning values to lua
    template<typename F>
    void Register(const std::string& name, F&& f);
    
    void exec(const std::string& code);
    
    LuaScript();
    LuaScript(const std::string& fileName);
    virtual ~LuaScript();
private:
    template<typename R, typename T> using mptr_t = R(T::*);
    void SetupBinding();
    std::unordered_map<std::string, std::pair<void*, Type>> propertyMap;
    std::unordered_map<std::string, std::unique_ptr<LuaFunctionBase>> methodMap;
};

//Do type checking, passing the wrong type can cause crashes
//TODO: automatically detect type
template<typename T>
void LuaScript::Register(const std::string& name, T& val, Type type) {
    propertyMap.insert({name, {&val, type}});
}

template<typename F>
void LuaScript::Register(const std::string& name, F&& f) {
    LUA_STACK_CHECK_START
    auto wrapped_function = wrap_lua_function(std::forward<F>(f));
    methodMap.emplace(name, std::make_unique<LuaFunction<function_type_t<decltype(wrapped_function)>>>(wrapped_function));
    
    //Create a table
    lua_createtable(L, 0, 0);
    auto table_idx = lua_gettop(L);
    
    //Create the metatable
    lua_createtable(L, 0, 0);
    auto metatable_idx = lua_gettop(L);
    lua_pushstring(L, "__call");
    
    lua_pushstring(L, name.c_str());
    lua_pushlightuserdata(L, &methodMap);
    lua_pushcclosure(L, call_cpp, 2);
    
    lua_settable(L, metatable_idx);

    lua_setmetatable(L, table_idx);
    
    lua_setglobal(L, name.c_str());
    LUA_STACK_CHECK_END
}
}
