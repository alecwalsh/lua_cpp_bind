#pragma once
// #include <utility>
// #include <string>
#include <new>
#include <functional>

#include "LuaScript.h"

#include "function_type_utils.h"
#include "function_wrapper.h"

namespace LuaCppBind {

template<typename T>
struct constructor_base {
    virtual void construct(lua_State*, T*) = 0;
};

template<typename T, typename... Args>
struct constructor : public constructor_base<T> {
    template<std::size_t... Is>
    void construct_impl(lua_State* L, T* t, std::index_sequence<Is...>) {
        new(t) T{
            //Cast the arguments from the types returned from Lua to the types accepted by the constructor
            static_cast<pack_element_t<Is, pack<Args...>>>(
                //Get the arguments from Lua
                LuaValue{L, Is+1}.get<::detail::corresponding_type_t<pack_element_t<Is, pack<Args...>>>>()
            )...
        };
    }
    
    
    void construct(lua_State* L, T* t) override {
        construct_impl(L, t, std::index_sequence_for<Args...>{});
    }
};

//Creates a new userdata for the objects, and calls the object's constructor
template<typename T/*, typename F*/>
int new_object(lua_State* L/*, std::function<void(lua_State*, int)> f, F&& get_args*/) {
//     auto L = ls.L;
    T* ud_addr = (T*)lua_newuserdata(L, sizeof(T));
    int ud_idx = lua_gettop(L);
    
//     make_from_tuple_placement(ud1, get_args(L));
    static_cast<constructor_base<T>*>(lua_touserdata(L, lua_upvalueindex(1)))->construct(L, ud_addr);
    
    //Create the metatable
    lua_newtable(L);
    int metatable_idx = lua_gettop(L);
    
    //Add the __gc metamethod
    lua_pushcfunction(L, [](lua_State* L){
        static_cast<T*>(lua_touserdata(L, 1))->~T();
        return 0;
    });
    lua_setfield(L, metatable_idx, "__gc");
    
    lua_newtable(L);
    //Add any methods for the userdata type here
//     f(L, lua_gettop(L));
    
    //Set __index to the table of methods
    lua_setfield(L, metatable_idx, "__index");
    
    lua_setmetatable(L, ud_idx);
    
    //Return the userdata
    return 1;
}
}
