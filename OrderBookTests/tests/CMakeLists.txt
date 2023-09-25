include_directories(${gtest_SOURCE_DIR}/include $gtest_SOURCE_DIR)
add_executable(RunTests OrderBookTests.cpp MatchingEngineTests.cpp)
target_link_libraries(RunTests gtest gtest_main OrderBook)