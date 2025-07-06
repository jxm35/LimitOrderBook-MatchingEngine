find_package(pybind11 REQUIRED)

pybind11_add_module(_mdfeed_receiver bindings.cpp)

set_target_properties(_mdfeed_receiver PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../orderbook
        OUTPUT_NAME "_mdfeed_receiver"
)
target_link_libraries(_mdfeed_receiver PRIVATE MDFeed)

target_compile_definitions(_mdfeed_receiver PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
target_compile_features(_mdfeed_receiver PRIVATE cxx_std_23)
