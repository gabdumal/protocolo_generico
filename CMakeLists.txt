# Version
cmake_minimum_required(VERSION 3.12...3.28)

if (${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif ()

# Project
project(dcc042
        LANGUAGES CXX
)

# Environment
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=-*,readability-identifier-naming")

# Directories
add_subdirectory(src)
add_subdirectory(external)
