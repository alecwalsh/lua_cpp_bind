#include <utility>
#include <type_traits>

#include "LuaTypes.h"

namespace LuaCppBind {

class lua_any_container_base {
public:
    virtual void* get() = 0;
};


template<typename T>
class lua_any_container : public lua_any_container_base {
    T t;
    LuaType type;
public:
    lua_any_container(T t) : t{t}, type{type_to_lua_type_v<T>} {}
    
    void* get() override {
        return &t;
    }
};

class lua_any {
    lua_any_container_base* container;
    
public:
    template<typename T>
    lua_any(T&& t) {
        container = new lua_any_container<std::remove_reference_t<T>>{std::forward<T>(t)};
    }
    
    template<typename T>
    T get() {
        return *static_cast<T*>(container->get());
    }
};

}
