cmake_minimum_required(VERSION 3.8.0) # 3.8 for cxx_standard

project(lua_cpp_bind)

file(GLOB SOURCES "*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/test.cpp")
 
add_library(lua_cpp_bind ${SOURCES})

target_compile_features(lua_cpp_bind PUBLIC cxx_std_17)
set_target_properties(lua_cpp_bind PROPERTIES CXX_STANDARD_REQUIRED true)

target_compile_options(lua_cpp_bind PRIVATE "-Wall")
target_include_directories(lua_cpp_bind PUBLIC "include")
target_link_libraries(lua_cpp_bind lua)

add_executable(lua_cpp_bind_test ${SOURCES} "test.cpp")
target_link_libraries(lua_cpp_bind_test lua_cpp_bind)

#Debug flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(lua_cpp_bind PUBLIC -DDEBUG)
endif()
