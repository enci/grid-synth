cmake_minimum_required(VERSION 3.21)
project(GridSynth VERSION 0.1.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(GRID_SYNTH_BUILD_TESTS "Build test applications" OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# External dependencies
include(FetchContent)

# SDL3
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main  # Use appropriate tag for SDL3
)
FetchContent_MakeAvailable(SDL3)

# Dear ImGui (using a fork with SDL3 support)
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

# Create ImGui library
file(GLOB IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/*.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC SDL3::SDL3)

# Add JSON library (Option 1 - FetchContent, preferred)
include(FetchContent)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# visit_struct
FetchContent_Declare(
        visit_struct
        GIT_REPOSITORY https://github.com/cbeck88/visit_struct.git
        GIT_TAG master
)
FetchContent_MakeAvailable(visit_struct)

# Add Portable File Dialogs library
FetchContent_Declare(
        pfd
        GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git
        GIT_TAG main  # You might want to use a specific tag/commit for stability
)
FetchContent_MakeAvailable(pfd)


# Source files
set(SOURCES
        source/main.cpp
        source/core/grid_synth.hpp
        source/core/grid_synth.cpp
        source/editor/editor.hpp
        source/editor/editor.cpp
)

# Main executable
add_executable(grid_synth ${SOURCES})

target_include_directories(grid_synth PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${pfd_SOURCE_DIR}  # This is the include path for portable-file-dialogs
)

# Define a preprocessor macro to enable Portable File Dialogs
target_compile_definitions(grid_synth PRIVATE USE_PORTABLE_FILE_DIALOGS)

target_link_libraries(grid_synth PRIVATE
        SDL3::SDL3
        imgui_lib
        nlohmann_json::nlohmann_json
)
