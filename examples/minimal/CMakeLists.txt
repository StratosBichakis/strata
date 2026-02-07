add_executable(str_rtsine
	rtsine.cpp
)
target_link_libraries(str_rtsine PRIVATE stk)
target_include_directories(str_rtsine PRIVATE ${STK_INCLUDE_DIR_LIST})

add_executable(str_rtfile
	rtfile.cpp
)
target_link_libraries(str_rtfile PRIVATE stk)
target_include_directories(str_rtfile PRIVATE ${STK_INCLUDE_DIR_LIST})

add_executable(str_rtblit
	rtblit.cpp
)
target_link_libraries(str_rtblit PRIVATE stk)
target_include_directories(str_rtblit PRIVATE ${STK_INCLUDE_DIR_LIST})

add_executable(str_bethree
	bethree.cpp
)
target_link_libraries(str_bethree PRIVATE stk)
target_include_directories(str_bethree PRIVATE ${STK_INCLUDE_DIR_LIST})

add_executable(str_controlbee-cli
	controlbee-cli.cpp
)
target_link_libraries(str_controlbee-cli PRIVATE stk)
target_include_directories(str_controlbee-cli  PRIVATE ${STK_INCLUDE_DIR_LIST})

add_executable(str_controlbee-srv
	controlbee-srv.cpp
)
target_link_libraries(str_controlbee-srv PUBLIC stk)
target_include_directories(str_controlbee-srv PUBLIC ${STK_INCLUDE_DIR_LIST})
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
	target_compile_definitions(str_controlbee-srv PUBLIC __MACOSX_CORE__)
endif ()

add_executable(str_controlbee-midi
	controlbee-midi.cpp
)

target_include_directories(str_controlbee-midi PRIVATE ${STK_INCLUDE_DIR_LIST})
target_link_libraries(str_controlbee-midi PRIVATE stk )

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
target_compile_definitions(str_controlbee-midi PUBLIC __MACOSX_CORE__)
endif ()