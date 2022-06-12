cmake_minimum_required(VERSION 3.5)

project(attribute LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(attribute INTERFACE)
target_include_directories(attribute INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(attribute_demo main.cpp)
target_include_directories(attribute_demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(attribute_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/utils/include)
