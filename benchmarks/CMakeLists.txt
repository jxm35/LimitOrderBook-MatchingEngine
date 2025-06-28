project(benchmarks)

include(FetchContent)
FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
)
FetchContent_MakeAvailable(googlebenchmark)
enable_testing()

add_executable(Benchmarks OrderBookBenchmarks.cpp)
target_link_libraries(Benchmarks OrderBook MDFeed benchmark::benchmark)
