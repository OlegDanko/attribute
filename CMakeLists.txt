cmake_minimum_required(VERSION 3.5)

project(attribute LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(attribute main.cpp)
target_include_directories(attribute PUBLIC include)
target_include_directories(attribute PRIVATE deps/utils/include)
