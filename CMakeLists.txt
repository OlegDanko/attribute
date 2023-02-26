cmake_minimum_required(VERSION 3.5)

project(attribute LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC
    src/GameState/GameStateClient.cpp
)

add_library(attribute ${SRC})
target_include_directories(attribute PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(attribute PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/utils/include)

add_executable(attribute_demo main.cpp ${SRC})
target_include_directories(attribute_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(attribute_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/utils/include)
