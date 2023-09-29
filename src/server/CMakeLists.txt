add_executable(WebSocketServer main.cpp session.cpp listener.cpp util.cpp)

# Link libraries
target_link_libraries(WebSocketServer PRIVATE)
target_link_libraries(WebSocketServer fmt::fmt)
target_link_libraries(WebSocketServer spdlog::spdlog)