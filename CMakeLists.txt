cmake_minimum_required(VERSION 3.10)
project(ClassProject3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)


include_directories(include)

file(GLOB SOURCES "src/*.cpp")


add_executable(${PROJECT_NAME} ${SOURCES})

add_compile_options(-Wall -Wextra -Werror)
set(SANITIZER -fsanitize=address,undefined -fno-omit-frame-pointer)
target_compile_options(${PROJECT_NAME} PRIVATE ${SANITIZER})
target_link_options(${PROJECT_NAME} PRIVATE ${SANITIZER})