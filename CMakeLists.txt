cmake_minimum_required(VERSION 4.2)

# Set C++ standard to C23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Tell clang which files are to be compiled into exec
project(main LANGUAGES CXX)
add_executable(${PROJECT_NAME} src/main.cpp)

# Raylib can be compiled into a dynamic library instead of being built
# directly into our exec, allowing for faster incremental compilation
add_subdirectory(includes/raylib)
include_directories("includes/raylib/src")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
# We'll do the same with entt
add_subdirectory(includes/entt)
include_directories("includes/entt/src")
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)

# Raygui works a little differently, it builds on raylib so it has to be
# integrated into our program as an include
target_include_directories(${PROJECT_NAME} PRIVATE "includes/raygui/src")
