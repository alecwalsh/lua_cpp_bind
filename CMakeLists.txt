cmake_minimum_required(VERSION 3.1.0)

project(lua_cpp_bind)

file(GLOB SOURCES "*.cpp")
 
add_executable(lua_cpp_bind ${SOURCES})

target_compile_features(lua_cpp_bind PUBLIC cxx_std_17)
set_target_properties(lua_cpp_bind PROPERTIES CXX_STANDARD_REQUIRED true)

target_compile_options(lua_cpp_bind PRIVATE "-Wall")
target_include_directories(lua_cpp_bind PUBLIC ".")
target_link_libraries(lua_cpp_bind lua)

#Debug flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(lua_cpp_bind PUBLIC -DDEBUG)
endif()
