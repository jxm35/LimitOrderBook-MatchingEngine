project(OrderBookTests)

add_subdirectory(tests)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        SomeTests
        tests/OrderBookTests.cpp
)
target_link_libraries(
        SomeTests
        OrderBook
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(SomeTests)