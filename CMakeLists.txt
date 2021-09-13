cmake_minimum_required(VERSION 3.15)

project(pbrt3)

file(GLOB CORE_EXE
    core/*.cpp
)

add_executable(pbrt3 "pbrt.cpp" ${CORE_EXE})
target_include_directories(pbrt3 PUBLIC core)