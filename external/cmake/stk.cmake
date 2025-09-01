include(FetchContent)
FetchContent_Populate(
  stk
  GIT_REPOSITORY https://github.com/StratosBichakis/stk.git
  GIT_TAG        e76073f926ec2ffec90124ef51222246831cd4ea # bela-rtAudio branch
)

cmake_path(SET STK_DIR ${stk_SOURCE_DIR})
cmake_path(SET STK_SOURCE_DIR ${STK_DIR}/src)
cmake_path(SET STK_INCLUDE_DIR ${STK_DIR}/include)

# get_filename_component(BELA_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
configure_file(${CMAKE_CURRENT_LIST_DIR}/stk-config.h.in ${CMAKE_BINARY_DIR}/external/stk-config.h @ONLY)

file(GLOB STK_SOURCES
	${STK_SOURCE_DIR}/*.cpp
	)

add_library(stk SHARED ${STK_SOURCES})
target_compile_features(stk PRIVATE cxx_std_11)

#target_compile_definitions(stk PUBLIC __LINUX_BELA__ __RTMIDI_DEBUG__)
target_compile_definitions(stk PUBLIC __LINUX_BELA__)

include(bela-lib)

target_link_directories(stk PUBLIC ${BELA_SYSROOT_PATH_LIST})
target_link_libraries(stk PUBLIC ${BELA_SYSROOT_LIB_LIST} ${BELA} ${BELA_EXTRA})

target_include_directories(stk PUBLIC
	${STK_INCLUDE_DIR}
	PRIVATE
	${BELA_INCLUDE_DIR}
)