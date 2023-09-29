cmake_minimum_required(VERSION 3.20)

project(qcustomplot LANGUAGES CXX)

find_package(Qt6 COMPONENTS Widgets PrintSupport REQUIRED)

add_library(qcustomplot SHARED)
target_compile_definitions(qcustomplot PRIVATE QCUSTOMPLOT_COMPILE_LIBRARY)
target_include_directories(qcustomplot PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_sources(qcustomplot PRIVATE ${CMAKE_CURRENT_LIST_DIR}/qcustomplot.cpp)
target_link_libraries(qcustomplot PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)