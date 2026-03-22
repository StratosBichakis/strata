add_executable(bang-srv
        bang-srv.cpp
)
target_link_libraries(bang-srv PUBLIC stk)
target_include_directories(bang-srv PUBLIC ${STK_INCLUDE_DIR_LIST})

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_compile_definitions(bang-srv PUBLIC __MACOSX_CORE__)
endif ()