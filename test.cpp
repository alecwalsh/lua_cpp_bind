#include "LuaScript.h"
#include "LuaValue.h"
#include "LuaTable.h"
#include "LuaUserdata.h"

#include "function_wrapper.h"

#include "Object.h"

#include "gtest/gtest.h"

#include "LuaCast.h"

#include <string>
#include <iostream>

using namespace LuaCppBind;

int fn(int i) {
    std::cout << i << std::endl;
    return i;
}

class LuaScriptFixture : public ::testing::Test {
protected:
    LuaScript ls{"../test.lua"};
    lua_State* L = ls.L;
};

TEST_F(LuaScriptFixture, ReadFromTable) {
    LUA_STACK_CHECK_START
    
    LuaTable table{ls, "table_test1"};
    
    ASSERT_TRUE(table["abc"].get<std::string>() == "def");
    ASSERT_TRUE(table[0].get<double>() == 0);
    ASSERT_TRUE(table[1].get<double>() == 1.5);
    
    LUA_STACK_CHECK_END
}

TEST_F(LuaScriptFixture, LambdaCapture) {
    LUA_STACK_CHECK_START
    
    int i = 0;
    
    ls.Register("lambda_test", [&i](const double& d) {
        i = d;
    });
    
    ls.exec("lambda_test(1)");
    ASSERT_TRUE(i == 1);
    
    LUA_STACK_CHECK_END
}

TEST_F(LuaScriptFixture, VariableRegistration) {
    LUA_STACK_CHECK_START
    auto s = "abcd";
    ls.Register("var_test1", s, LuaType::String);
    
    ls.exec("return cpp.var_test1");
    std::string s2 = lua_tostring(L, -1);
    lua_pop(L, 1);
    
    ASSERT_EQ(s2, s);
    LUA_STACK_CHECK_END
}

TEST_F(LuaScriptFixture, ObjectCreation) {
//     LuaObject<Object>{ls, "object"}
//     .RegisterConstructor<int, bool>()
//     .RegisterMethod("printi", &Object::printi)
//     .Finalize();
//     
//     ls.exec("new_object(2, true):printi()");
}

TEST(LuaCast, CastTest) {
    lua_any a = 5;
    ASSERT_EQ(a.get<int>(), 5);
    a = 5.6;
    ASSERT_EQ(a.get<double>(), 5.6);
}

//TODO: add proper unit tests
int main(int argc, char* argv[]) {
    LuaScript ls{"../test.lua"};
    
    auto table = LuaTable{ls, "table_test1"};
        
    auto l = [](int i){std::cout << "Hello from Lua " << i << std::endl;};
    
    ls.Register("lambda_test", []{
        std::cout << "Hello from Lua" << std::endl;
    });
    ls.Register("lambda_test2", [](const double& i, bool j){
        std::cout << "Hello from Lua " << i << std::endl;
    });
    ls.Register("lambda_test3", l);
    ls.Register("function_test", fn);
    
    auto s = "abcd";
    ls.Register("var_test", s, LuaType::String);
    
    LuaObject<Object>{ls, "object"}
    .RegisterConstructor<int, bool>()
    .RegisterMethod("printi", &Object::printi)
    .Finalize();
    
    ls.exec("call_cpp()");
    for(const auto [t1, t2] : table) {
        std::cout << t1 << " => " << t2 << std::endl;
    }
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
