cmake_minimum_required(VERSION 3.15)

project(pbrt3)

# add fmt lib
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd/fmt)

file(GLOB CORE_SOURCE
    core/*.cpp
)

file(GLOB INTEGRATOR_SOURCE
    integrators/*.cpp
)

file(GLOB SHAPE_SOURCE
    shapes/*.cpp
)

add_executable(pbrt3 "pbrt.cpp" ${CORE_SOURCE} ${INTEGRATOR_SOURCE} ${SHAPE_SOURCE})
target_include_directories(pbrt3 PUBLIC core integrators shapes)
target_link_libraries(pbrt3 PUBLIC fmt)

# test
add_executable(test "test.cpp" "core/geometry.cpp")
target_include_directories(test PUBLIC core)
target_link_libraries(test PUBLIC fmt)
