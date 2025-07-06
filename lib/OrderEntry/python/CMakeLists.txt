find_package(pybind11 REQUIRED)

pybind11_add_module(_orderentry_client bindings.cpp)

set_target_properties(_orderentry_client PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../orderbook
        OUTPUT_NAME "_orderentry_client"
)
target_link_libraries(_orderentry_client PRIVATE OrderEntry)

target_compile_definitions(_orderentry_client PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
target_compile_features(_orderentry_client PRIVATE cxx_std_23)
