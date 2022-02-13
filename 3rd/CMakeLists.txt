cmake_minimum_required(VERSION 3.15)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/glfw)

# set imgui sources
set(IMGUI_ROOT "${CMAKE_CURRENT_LIST_DIR}/imgui")
file(GLOB imgui_sources "${IMGUI_ROOT}/*.cpp")
list(APPEND glfw_gl_imp_sources "${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp" "${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp")

add_library(imgui STATIC
    "${imgui_sources}"
    "${glfw_gl_imp_sources}"
)

target_include_directories(imgui 
    PUBLIC
        "${IMGUI_ROOT}"
        "${IMGUI_ROOT}/backends"
)

target_link_libraries(imgui PUBLIC glfw)