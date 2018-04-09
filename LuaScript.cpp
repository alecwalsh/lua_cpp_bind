#include <iostream>
#include <cassert>

#include "LuaScript.h"
#include "LuaFunction.h"

namespace LuaCppBind {

LuaScript::LuaScript() {
    L = luaL_newstate();
    luaL_openlibs(L);
    
    //Push error handler
    lua_pushcfunction(L, [](lua_State *L) {
            //Prints the error message and returns the error object unchanged
            const char *msg = lua_tostring(L, -1);
            if(msg != NULL) {
                printf("Lua error: %s\n", msg);
            }
            return 1;
        });
    
    SetupBinding();
}

LuaScript::LuaScript(std::string fileName) : LuaScript() {
    int err = luaL_loadfile(L, fileName.c_str());
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
    int err = luaL_loadstring(L, code.c_str());
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
    
    //Create two tables.  One will be a regular table and the other will be its metatable
    lua_newtable(L);
    lua_newtable(L);
    
    //Push a pointer to propertyMap onto the stack.  Duplicate it because we call lua_pushcclosure twice
    lua_pushlightuserdata(L, &propertyMap);
    lua_pushvalue(L, -1);
    
    //Set the __newindex and __index metamethods
    lua_pushcclosure(L, [](lua_State* L) {
            //This function takes 3 arguments, but set_cpp only needs the 2nd and 3rd
            lua_remove(L, -3);
            return set_cpp(L);
        }, 1);
    lua_setfield(L, -3, "__newindex");
    
    lua_pushcclosure(L, [](lua_State* L){
            //This function takes 2 arguments, but get_cpp only needs the 2nd
            lua_remove(L, -2);
            return get_cpp(L);
        }, 1);
    lua_setfield(L, -2, "__index");
    
    //Now set the table at the top of the stack as the metatable of the other one, then give it a name
    lua_setmetatable(L, -2);
    lua_setglobal(L, "cpp");
    
    LUA_STACK_CHECK_END
}

int call_cpp(lua_State* L) {
    const char* name = lua_tostring(L, lua_upvalueindex(1));
    int methodMap_idx = lua_upvalueindex(2);
        
    auto& methodMap = *static_cast<decltype(LuaScript::methodMap)*> (lua_touserdata(L, methodMap_idx));
    methodMap[name]->apply(L);
    
    return 0;
}

//TODO: add error handling
int set_cpp(lua_State* L) {
    int propertyMap_idx = lua_upvalueindex(1);
    const char* name = lua_tostring(L, 1);
    
    auto& propertyMap = *static_cast<decltype(LuaScript::propertyMap)*>(lua_touserdata(L, propertyMap_idx));
    
    if(!propertyMap.count(name)) {
        printf("Error: Name %s has not been registered\n", name);
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
        case Type::Float:
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
    
    return 0;
}

int get_cpp(lua_State* L) {
    int propertyMap_idx = lua_upvalueindex(1);
    const char* name = lua_tostring(L, 1);
    
    auto& propertyMap = *static_cast<decltype(LuaScript::propertyMap)*>(lua_touserdata(L, propertyMap_idx));
    
    if(!propertyMap.count(name)) {
        printf("Error: Name %s has not been registered\n", name);
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
        case Type::Float:
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
    
    return 1;
}
}
