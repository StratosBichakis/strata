# Add the executable
add_executable(file-player file-player.cpp FPlayer.cpp)
target_link_libraries(file-player PRIVATE stk)
target_include_directories(file-player PRIVATE ${STK_INCLUDE_DIR_LIST})

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_compile_definitions(file-player PUBLIC __MACOSX_CORE__)
endif ()