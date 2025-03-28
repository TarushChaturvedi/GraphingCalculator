cmake_minimum_required(VERSION 3.14)
project(GraphingCalculator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs)

add_executable(${PROJECT_NAME} src/main.cpp src/parser.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-system
    sfml-graphics
    sfml-window
    ImGui-SFML::ImGui-SFML
)

target_include_directories(${PROJECT_NAME} PRIVATE "${imgui_SOURCE_DIR}/misc/cpp")