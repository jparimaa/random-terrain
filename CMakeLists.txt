cmake_minimum_required(VERSION 3.5)
project(random-terrain VERSION 0.0.1 LANGUAGES C CXX)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "64bit is required")
endif()

add_library(glad ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include/glad/glad.h ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c)
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include")

set(GLFW_PATH CACHE PATH "Path to GLFW")

file(GLOB SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

set(EXE_NAME random-terrain)
add_executable(${EXE_NAME} ${SOURCES})

target_compile_features(${EXE_NAME} PUBLIC cxx_std_11)
target_compile_definitions(${EXE_NAME} PUBLIC -D_CRT_SECURE_NO_WARNINGS -DSHADER_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")

target_include_directories(${EXE_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/stb"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/glm"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include"
        "${GLFW_PATH}/include"
)

target_link_libraries(${EXE_NAME}
PUBLIC    
    glad
    opengl32.lib
    "${GLFW_PATH}/lib-vc2015/glfw3.lib"
)

FIND_PACKAGE(Threads)
TARGET_LINK_LIBRARIES(${EXE_NAME} ${CMAKE_THREAD_LIBS_INIT})

if(MSVC)
    target_compile_options(${EXE_NAME} PRIVATE /W3 /WX /MP)
else()
    target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra -Werror)
endif()
