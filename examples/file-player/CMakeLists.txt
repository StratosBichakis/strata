# Add the executable
add_executable(file-player file-player.cpp FPlayer.cpp)

# target_include_directories(file-player PRIVATE ${BELA_INCLUDE_DIR} ${BELA_SYSROOT_PATH}/usr/include)
# target_link_directories(file-player PRIVATE ${BELA_SYSROOT_PATH_LIST})
target_link_libraries(file-player PRIVATE stk)