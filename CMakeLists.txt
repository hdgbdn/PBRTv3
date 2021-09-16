cmake_minimum_required(VERSION 3.15)

project(pbrt3)

file(GLOB CORE_SOURCE
    core/*.cpp
)

file(GLOB INTEGRATOR_SOURCE
    integrators/*.cpp
)

add_executable(pbrt3 "pbrt.cpp" ${CORE_SOURCE} ${INTEGRATOR_SOURCE})
target_include_directories(pbrt3 PUBLIC core integrators)
