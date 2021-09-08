cmake_minimum_required(VERSION 3.20)
project(demo)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Looking for lld")
find_program(LLD_FOUND lld)
if (LLD_FOUND)
    message(STATUS "Looking for lld - found")
    set(CMAKE_C_LINKER_FLAGS "-fuse-ld=lld")
    set(CMAKE_CXX_LINKER_FLAGS "-fuse-ld=lld")
    set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
else()
    message(STATUS "Looking for lld - not found")
endif()

FetchContent_Declare(
    pivot
    GIT_REPOSITORY https://github.com/EIP-Pivot/pivot.git
    GIT_TAG dc7eb95f5ad2df01f9f1ad349e8a5ed05bf95e91
)
if (NOT pivot_POPULATED)
    message(STATUS "Populating pivot")
    FetchContent_Populate(pivot)
    add_subdirectory(${pivot_SOURCE_DIR} ${pivot_BINARY_DIR})
    target_compile_definitions(pivot-graphics PUBLIC
        MAX_PROJECTION_LIMIT=1000.0f
    )
endif()

add_executable(${PROJECT_NAME} source/main.cxx source/Systems/RenderableSystem.cxx)

target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_link_libraries(${PROJECT_NAME} PRIVATE pivot-ecs pivot-graphics)
