#include "LuaScript.h"
#include "LuaValue.h"
#include "LuaTable.h"
#include "LuaUserdata.h"

#include "function_wrapper.h"

#include "Object.h"

#include "gtest/gtest.h"


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
    LuaTable table{ls, "gl_version"};
};

TEST_F(LuaScriptFixture, ReadFromTable) {
    ASSERT_TRUE(table["abc"].get<std::string>() == "def");
    ASSERT_TRUE(table[0].get<double>() == 0);
    ASSERT_TRUE(table[1].get<double>() == 1.5);
}

//TODO: add proper unit tests
int main(int argc, char* argv[]) {
    LuaScript ls{"../test.lua"};
    
    auto table = LuaTable{ls, "gl_version"};
        
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
