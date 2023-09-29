project(tests)

# include gTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(Tests OrderBookTests.cpp MatchingEngineTests.cpp OrderBookEntryTests.cpp)
target_link_libraries(Tests OrderBook GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(Tests)

include_directories(${gtest_SOURCE_DIR}/include $gtest_SOURCE_DIR)