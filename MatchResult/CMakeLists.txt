set(HEADER_FILE MatchResult.h)
set(SOURCE_FILES MatchResult.cpp)

add_library(MatchResult STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(MatchResult PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
#target_link_libraries(Order OrderCore)

# You can link additional libraries if needed
# target_link_libraries(derived_class some_other_library)