cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS -DGLEW_STATIC )

project(pogla)

find_package(GLEW)
find_package(GLUT)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

add_executable(main main.cpp camera.cpp)

target_link_libraries(main -lGL  -lGLEW -lglut -lpthread -lGLU -lm)

target_compile_options(main PRIVATE -Wextra -Wall)
