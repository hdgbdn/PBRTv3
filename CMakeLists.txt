cmake_minimum_required(VERSION 3.15)

project(pbrt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# add fmt lib
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd/fmt)

# add imgui lib
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd)

file(GLOB PBRT_SOURCES
    accelerators/*.cpp
    core/*.cpp
    integrators/*.cpp
    shapes/*.cpp
    cameras/*.cpp
    materials/*.cpp
    samplers/*.cpp
    filters/*.cpp
    textures/*.cpp
    lights/*.cpp
)

add_executable(${CMAKE_PROJECT_NAME} "pbrt.cpp" 
                ${PBRT_SOURCES} 
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC fmt imgui OpenGL::GL)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 3rd/stbimage)

if (MSVC)

else()
    # MinGW compiler on Window, link C&C++ standard libraies statically
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC -static)
endif()



# test
add_executable(test "test.cpp" "core/parallel.cpp" "core/stats.cpp" )
target_include_directories(test PUBLIC core)
target_link_libraries(test PUBLIC fmt)
