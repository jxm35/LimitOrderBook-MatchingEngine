find_package(pybind11 REQUIRED)

pybind11_add_module(_orderbook bindings.cpp)

set_target_properties(_orderbook PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../orderbook
        OUTPUT_NAME "_orderbook"
)
target_link_libraries(_orderbook PRIVATE OrderBook)

target_compile_definitions(_orderbook PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
target_compile_features(_orderbook PRIVATE cxx_std_23)
