project(benchmarks)

# include gTest
include(FetchContent)
#FetchContent_Declare(
#        googletest
#        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
#)
FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(googlebenchmark)
enable_testing()

add_executable(Benchmarks OrderBookBenchmarks.cpp)
target_link_libraries(Benchmarks OrderBook benchmark::benchmark)
#include(GoogleTest)
#gtest_discover_tests(Benchmarks)

#include_directories(${gtest_SOURCE_DIR}/include $gtest_SOURCE_DIR)