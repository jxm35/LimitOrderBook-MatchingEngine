cmake_minimum_required(VERSION 3.24)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 20)

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

# fmt setup
set(FMT_ROOT /opt/homebrew/Cellar/fmt/10.1.1)
find_package(fmt REQUIRED)
include_directories(/opt/homebrew/Cellar/fmt/10.1.1/include)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tests)
