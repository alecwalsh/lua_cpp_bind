#cmake_minimum_required(VERSION 3.8.0) # 3.8 for cxx_standard
#Reenable above when Debian has CMake 3.8
cmake_minimum_required(VERSION 3.7.0)

project(lua_cpp_bind)

set(BASE "/build")

file(GLOB SOURCES "*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/test.cpp")

link_directories(${BASE}/lib)

add_library(lua_cpp_bind ${SOURCES})

target_compile_features(lua_cpp_bind PUBLIC cxx_std_17)
set_target_properties(lua_cpp_bind PROPERTIES CXX_STANDARD_REQUIRED true)

target_compile_options(lua_cpp_bind PRIVATE "-Wall")
target_include_directories(lua_cpp_bind PUBLIC "include")
target_link_libraries(lua_cpp_bind lua)
if(MINGW)
    set_target_properties(lua_cpp_bind PROPERTIES LINK_FLAGS "-static")
    set(BASE "/build")
    target_include_directories(lua_cpp_bind PUBLIC ${BASE}/include)
endif()

add_executable(lua_cpp_bind_test ${SOURCES} "test.cpp")
target_compile_options(lua_cpp_bind_test PRIVATE "-Wall")
target_link_libraries(lua_cpp_bind_test lua_cpp_bind)

#Debug flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(lua_cpp_bind PUBLIC -DDEBUG)
endif()
