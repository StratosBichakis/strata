# Add the executable
add_executable(file-player file-player.cpp FPlayer.cpp)
target_link_libraries(file-player PRIVATE stk)

if(NOT BUILD_FOR_BELA)
    target_compile_definitions(file-player PUBLIC __MACOSX_CORE__)
    target_link_libraries(latelybass PRIVATE ${stk-lib} ${Foundation-lib} ${AudioToolbox-lib} ${pthread-lib})
endif ()