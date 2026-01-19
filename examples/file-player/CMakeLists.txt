# Add the executable
add_executable(file-player file-player.cpp FPlayer.cpp)

target_link_libraries(file-player PRIVATE stk)