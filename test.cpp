#include "LuaScript.h"
#include "LuaValue.h"


int main() {
    LuaScript ls{"../test.lua"};
    
    table_t table = get_lua_table(ls, "table2");

    ls.exec("io.write(\"a\")");
    return 0;
}
