#pragma once
#include <unordered_map>
#include <functional>
#include <utility>

#include <cassert>

#include <lua.hpp>

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

class LuaScript {
protected:
public:
    lua_State* L;
    //The integer and float types are LUA_INTEGER and LUA_NUMBER from luaconf.h, make sure to use the right types(e.g. Don't use double instead of float)
    enum class Type {
        Integer,
        Float,
        String,
        Bool,
        Table
    };
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
    std::unordered_map<std::string, std::function<void()>> methodMap;
};

template<typename T>
void LuaScript::Register(std::string name, T ptr, Type type) {
    propertyMap.insert({name, {ptr, type}});
}

template<typename R, typename T, typename... Args>
void LuaScript::Register(std::string name, mptr_t<R,T> mptr, T* obj, Args&&... args) {
    //Creates a lambda that calls mptr on obj with args
    //Args is passed by reference, so you can change them after calling Register
    methodMap.insert({name, [=, &args...](){return (obj->*mptr)(std::forward<Args>(args)...);}});
    
    lua_pushstring(L, name.c_str());
    lua_pushlightuserdata(L, &methodMap);
    lua_pushcclosure(L, call_cpp, 2);
    lua_setglobal(L, name.c_str());
}

template<typename F>
void LuaScript::Register(std::string name, F&& f) {
    methodMap.insert({name, std::function<void()>(std::forward<F>(f))});
    
    lua_pushstring(L, name.c_str());
    lua_pushlightuserdata(L, &methodMap);
    lua_pushcclosure(L, call_cpp, 2);
    lua_setglobal(L, name.c_str());
}
