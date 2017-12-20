cmake_minimum_required(VERSION 3.9)
project(GPUBasedRendering_A3)

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp main.h global.h system.cpp system.h utility.cpp utility.h draw.cpp draw.h Shader.cpp Shader.h)

include_directories(include)

link_directories(lib)

add_executable(GPUBasedRendering_A3 ${SOURCE_FILES})

target_link_libraries(GPUBasedRendering_A3 ${OPENGL_LIBRARIES} glut32.lib glut.lib glew32.lib glew32s.lib)