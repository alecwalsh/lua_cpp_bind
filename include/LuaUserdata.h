#pragma once
// #include <utility>
// #include <string>
#include <new>
#include <functional>
#include <unordered_map>
#include <memory>

#include "LuaScript.h"
#include "LuaTypes.h"
#include "LuaMethod.h"

#include "function_type_utils.h"
#include "function_wrapper.h"


namespace LuaCppBind {

template<typename T>
struct constructor_base {
    virtual void construct(lua_State*, T*) = 0;
    virtual ~constructor_base() = default;
};

template<typename T, typename... Args>
struct constructor : public constructor_base<T> {
    template<std::size_t... Is>
    void construct_impl(lua_State* L, T* t, std::index_sequence<Is...>) {
        new(t) T {
            //Cast the arguments from the types returned from Lua to the types accepted by the constructor
            static_cast<pack_element_t<Is, Args...>>(
                //Get the arguments from Lua
                LuaValue{L, Is+1}.get<corresponding_type_t<pack_element_t<Is, Args...>>>()
            )...
        };
    }
    
    
    void construct(lua_State* L, T* t) override {
        construct_impl(L, t, std::index_sequence_for<Args...>{});
    }
};

template<typename T>
struct LuaObject {
    //TODO: Unify with LuaScript::call_cpp
    static int call_cpp(lua_State* L) {
        const char* name = lua_tostring(L, lua_upvalueindex(1));
        int methodMap_idx = lua_upvalueindex(2);
            
        auto& methodMap = *static_cast<decltype(LuaObject::methodMap)>(lua_touserdata(L, methodMap_idx));
        methodMap[name]->apply(L);
        
        return 0;
    }
    
    
    LuaObject(const LuaScript& ls, const std::string name) : ls{ls}, name{name} {
        methodMap = new std::unordered_map<std::string, std::unique_ptr<LuaMethodBase>>{};
        add_methods = new std::vector<std::unique_ptr<add_method_t>>{};
    }
    
    
    //TODO: Support multiple constructors
    template<typename... Args>
    [[nodiscard]] LuaObject& RegisterConstructor() {
        ctor = new constructor<T, Args...>{};
        return *this;
    }
    
    template<typename F>
    [[nodiscard]] LuaObject& RegisterMethod(std::string name, F&& f) {
        auto L = ls.L;
        LUA_STACK_CHECK_START
        
        //TODO: add wrap_lua_method
//         auto wrapped_function = wrap_lua_function(std::forward<F>(f));
        
//         methodMap.emplace(name, std::make_unique<LuaMethod<method_type_t<decltype(wrapped_function)>>>(wrapped_function));
        methodMap->emplace(name, std::make_unique<LuaMethod<method_type_t<F>>>(f));
        
        add_methods->emplace_back(std::make_unique<add_method_t>(
            // Can't capture member by value, so use lambda capture with initializer
            [name, methodMap=methodMap](lua_State* L, T* object){
                //This lambda 
                lua_pushstring(L, name.c_str());
                
                //Create a table
                lua_createtable(L, 0, 0);
                int table_idx = lua_gettop(L);
                
                //Create the metatable
                lua_createtable(L, 0, 0);
                int metatable_idx = lua_gettop(L);
                lua_pushstring(L, "__call");
                
                lua_pushstring(L, name.c_str());
                lua_pushlightuserdata(L, methodMap);
                lua_pushlightuserdata(L, object);
                lua_pushcclosure(L, call_cpp, 3);
                
                lua_settable(L, metatable_idx);

                lua_setmetatable(L, table_idx);
                
                lua_settable(L, -3);
            }
        ));
        
        LUA_STACK_CHECK_END
        return *this;
    }
    
    void Finalize() {
        lua_pushlightuserdata(ls.L, ctor);
        lua_pushlightuserdata(ls.L, add_methods);
        lua_pushlightuserdata(ls.L, methodMap);
        lua_pushcclosure(ls.L, LuaObject<T>::new_object, 3);
        //TODO: Use object.new instead of new_object
        lua_setglobal(ls.L, ("new_" + name).c_str());
    }
    
private:
    const LuaScript& ls;
    const std::string name;
    int ud_idx;
    constructor_base<T>* ctor;
    
    // Type for members of add_methods
    using add_method_t = std::function<void(lua_State*, T*)>;
    // These functions are called in new_object
    // They are responsible for registering each method with its object
    std::vector<std::unique_ptr<add_method_t>>* add_methods;
    
    std::unordered_map<std::string, std::unique_ptr<LuaMethodBase>>* methodMap;
    
    
    //Creates a new userdata for the objects, and calls the object's constructor
    //TODO: Crashes when called more than once
    static int new_object(lua_State* L) {
        T* ud_addr = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
        const int ud_idx = lua_gettop(L);
        
        auto ctor = static_cast<constructor_base<T>*>(lua_touserdata(L, lua_upvalueindex(1)));
        
        ctor->construct(L, ud_addr);
        
        auto* add_methods = static_cast<std::vector<std::unique_ptr<add_method_t>>*>(lua_touserdata(L, lua_upvalueindex(2)));
        
        //Create the metatable
        lua_newtable(L);
        int metatable_idx = lua_gettop(L);
        
        
        //This is allocated in the LuaObject constructor and needs to be deallocated in the userdata's __gc metamethod'
        auto* methodMap = static_cast<std::unordered_map<std::string, std::unique_ptr<LuaMethodBase>>*>(lua_touserdata(L, lua_upvalueindex(3)));
        
        //Add the __gc metamethod
        lua_pushlightuserdata(L, methodMap);
        lua_pushcclosure(L, [](lua_State* L) {
            delete static_cast<std::unordered_map<std::string, std::unique_ptr<LuaMethodBase>>*>(lua_touserdata(L, lua_upvalueindex(1)));
            
            static_cast<T*>(lua_touserdata(L, 1))->~T();
            return 0;
        }, 1);
        lua_setfield(L, metatable_idx, "__gc");
        
        
        lua_newtable(L);
        
        //Add all methods to the new instance
        for(const auto& f : *add_methods) {
            (*f)(L, ud_addr);
        }
        
        delete add_methods;
        delete ctor;
        
        //Set __index to the table of methods
        lua_setfield(L, metatable_idx, "__index");
        
        
        lua_setmetatable(L, ud_idx);
        
        //Return the userdata
        return 1;
    }
};

}
