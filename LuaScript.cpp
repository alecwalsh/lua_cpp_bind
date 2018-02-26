#include <lua.hpp>
#include <iostream>
#include <cassert>

#include "LuaScript.h"
#include "LuaFunction.h"

int error_handler(lua_State *L) {
    //Prints the error message and returns the error object unchanged
    const char *msg = lua_tostring(L, -1);
    if(msg != NULL) {
        printf("%s\n", msg);
    }
    return 1;
}

LuaScript::LuaScript() {
    L = luaL_newstate();
    luaL_openlibs(L);
    
    //Push error handler
    lua_pushcfunction(L, error_handler);
    
    SetupBinding();
}

LuaScript::LuaScript(std::string fileName) {
    L = luaL_newstate();
    luaL_openlibs(L);
    
    //Push error handler
    lua_pushcfunction(L, error_handler);
    
    SetupBinding();
    
    auto err = luaL_loadfile(L, fileName.c_str());
    if(err != LUA_OK) {
        switch(err) {
            case LUA_ERRSYNTAX:
                std::cout << "Syntax error in Lua file " << fileName << std::endl;
                break;
            case LUA_ERRFILE:
                std::cout << "Error opening " << fileName << std::endl;
                break;
            default:
                std::cout << "Error running Lua code" << std::endl;
                std::cout << "Error code: " << err << std::endl;
                break;
        }
        return;
    }
    lua_pcall(L, 0, LUA_MULTRET, 1);
}

LuaScript::~LuaScript() {
    lua_close(L);
}

void LuaScript::exec(std::string code) {
    auto err = luaL_loadstring(L, code.c_str());
    if(err != LUA_OK) {
        if(err == LUA_ERRSYNTAX) {
            std::cout << "Syntax error in Lua code: " << std::endl;
            std::cout << code << std::endl;
        } else {
            std::cout << "Error running Lua code" << std::endl;
            std::cout << "Error code: " << err << std::endl;
        }
        return;
    }
    lua_pcall(L, 0, LUA_MULTRET, 1);
}

void LuaScript::SetupBinding() {
    LUA_STACK_CHECK_START
    lua_pushlightuserdata(L, &propertyMap);
    lua_pushcclosure(L, set_cpp, 1);
    lua_setglobal(L, "set_cpp");
    
    lua_pushlightuserdata(L, &propertyMap);
    lua_pushcclosure(L, get_cpp, 1);
    lua_setglobal(L, "get_cpp");
    
    lua_newtable(L);
    lua_setglobal(L, "cpp");
    
    lua_newtable(L);
    lua_setglobal(L, "metacpp");
    
    luaL_dostring(L, R"(
        function metacpp.__newindex(table, key, value)
            set_cpp(key, value)
        end
    )");
    
    luaL_dostring(L, R"(
        function metacpp.__index(table, key)
            return get_cpp(key)
        end
    )");
    
    lua_getglobal(L, "cpp");
    lua_getglobal(L, "metacpp");
    
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
    LUA_STACK_CHECK_END
}

//TODO: Pass lua arguments to cpp
int call_cpp(lua_State* L) {
    LUA_STACK_CHECK_START
    auto name = lua_tostring(L, lua_upvalueindex(1));
    int methodMap_idx = lua_upvalueindex(2);
    
    std::cout << "Stack top = " << lua_gettop(L) << std::endl;
    
//     for(int i = 1; i <= lua_gettop(L); i++) {
//         std::cout << lua_typename(L, lua_type(L,i)) << std::endl;
//     }
        
    auto& methodMap = *static_cast<std::unordered_map<std::string, LuaFunctionAndTypes*>*> (lua_touserdata(L, methodMap_idx));
    methodMap[name]->func->apply(L);
    
    LUA_STACK_CHECK_END
    return 0;
}

//TODO: add error handling
int set_cpp(lua_State* L) {
    LUA_STACK_CHECK_START
    int propertyMap_idx = lua_upvalueindex(1);
    std::string name = lua_tostring(L, 1);
    
    
    auto& propertyMap = *static_cast<std::unordered_map<std::string, std::pair<void*, LuaScript::Type>>*>(lua_touserdata(L, propertyMap_idx));
    
    if(!propertyMap.count(name)) {
        printf("Error: Name has not been registered\n");
        return 0;
    }
    //A std::pair containg a pointer to the value and the type of the value
    auto& propertyValueType = propertyMap[name];
    
    switch(propertyValueType.second) {
        //TODO: handle tables
        using Type = LuaScript::Type;
//         case Type::Integer:
//             *((int*)propertyValueType.first) = lua_tointeger(L, 2);
//             break;
        case Type::Number:
            *((float*)propertyValueType.first) = lua_tonumber(L, 2);
            break;
        case Type::String:
            *((const char**)propertyValueType.first) = lua_tostring(L, 2);
            break;
        case Type::Bool:
            *((bool*)propertyValueType.first) = lua_toboolean(L, 2);
            break;
        case Type::Table:
            break;
        default:
            printf("Error: Invalid Lua type\n");
            break;
    }
    
    LUA_STACK_CHECK_END
    return 0;
}

int get_cpp(lua_State* L) {
    LUA_STACK_CHECK_START
    int propertyMap_idx = lua_upvalueindex(1);
    std::string name = lua_tostring(L, 1);
    lua_pop(L, 1);
    
    auto& propertyMap = *static_cast<std::unordered_map<std::string, std::pair<void*, LuaScript::Type>>*>(lua_touserdata(L, propertyMap_idx));
    
    if(!propertyMap.count(name)) {
        printf("Error: Name has not been registered\n");
        return 0;
    }
    //A std::pair containg a pointer to the value and the type of the value
    auto& propertyValueType = propertyMap[name];
    
    switch(propertyValueType.second) {
        //TODO: handle tables
        using Type = LuaScript::Type;
//         case Type::Integer:
//             lua_pushinteger(L, *((int*)propertyValueType.first));
//             break;
        case Type::Number:
            lua_pushnumber(L, *((float*)propertyValueType.first));
            break;
        case Type::String:
            lua_pushstring(L, *((const char**)propertyValueType.first));
            break;
        case Type::Bool:
            lua_pushboolean(L, *((bool*)propertyValueType.first));
            break;
        default:
            printf("Error: Invalid Lua type\n");
            break;
    }
    
    LUA_STACK_CHECK_END
    return 1;
}
