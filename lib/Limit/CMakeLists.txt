set(HEADER_FILE Limit.h)
set(SOURCE_FILES Limit.cpp)

add_library(Limit STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(Limit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
#target_link_libraries(Limit Order)

# You can link additional libraries if needed
# target_link_libraries(derived_class some_other_library)