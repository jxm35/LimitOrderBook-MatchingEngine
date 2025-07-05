cmake_minimum_required(VERSION 3.24)

if (EXISTS "/opt/homebrew/opt/llvm/bin/clang++")
    message(STATUS "Using Homebrew LLVM Clang")
    set(CMAKE_C_COMPILER /opt/homebrew/opt/llvm/bin/clang CACHE STRING "" FORCE)
    set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++ CACHE STRING "" FORCE)
else ()
    message(STATUS "Using system default compiler")
endif ()

project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif ()

# Boost setup
set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.82.0_1)
set(BOOST_INCLUDE_DIR /opt/homebrew/Cellar/boost/1.82.0_1/include)
set(BOOST_LIBRARY_DIR /opt/homebrew/Cellar/boost/1.82.0_1/lib)
find_package(Boost 1.82.0 COMPONENTS REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif ()

# spdlog setup
set(SPDLOG_ROOT /opt/homebrew/Cellar/spdlog/1.12.0)
find_package(spdlog REQUIRED)
include_directories(/opt/homebrew/Cellar/spdlog/1.12.0/include)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmarks)

option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
if (BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)
    if (pybind11_FOUND)
        message(STATUS "Building Python bindings")
        add_subdirectory(lib/OrderBook/python)
    else ()
        message(WARNING "pybind11 not found, skipping Python bindings")
    endif ()
endif ()
