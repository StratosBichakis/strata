add_executable(rtsine
	rtsine.cpp
)
target_link_libraries(rtsine PRIVATE stk)

add_executable(rtfile
	rtfile.cpp
)
target_link_libraries(rtfile PRIVATE stk)

add_executable(rtblit
	rtblit.cpp
)
target_link_libraries(rtblit PRIVATE stk)

add_executable(bethree
	bethree.cpp
)
target_link_libraries(bethree PRIVATE stk)

add_executable(controlbee-cli
	controlbee-cli.cpp
)
target_link_libraries(controlbee-cli PRIVATE stk)

add_executable(controlbee-srv
	controlbee-srv.cpp
)
target_link_libraries(controlbee-srv PRIVATE stk)

add_executable(controlbee-midi
	controlbee-midi.cpp
)
target_link_libraries(controlbee-midi PRIVATE stk)