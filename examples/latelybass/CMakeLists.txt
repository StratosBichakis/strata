# Add the executable
add_executable(latelybass latelybass.cpp LBass.cpp LBass.h)

target_link_libraries(latelybass PRIVATE stk)
target_include_directories(latelybass PRIVATE ${STK_INCLUDE_DIR_LIST})

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_compile_definitions(latelybass PUBLIC __MACOSX_CORE__)
endif ()
