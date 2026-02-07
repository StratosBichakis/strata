include(FetchContent)
FetchContent_Declare(
  	stk
  	GIT_REPOSITORY https://github.com/StratosBichakis/stk.git
  	GIT_TAG        e6281dc2d272f6e5b5e73d77ae7ad88fc0b4ee9d # bela-rtAudio branch
	GIT_SHALLOW TRUE
	SOURCE_SUBDIR     pathThatDoesNotExist #to block from add_subdirectory
)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(ENABLE_JACK OFF)
FetchContent_MakeAvailable(stk)

option(RTAUDIO_SAMPLE_RATE 44100.0)

if(BUILD_FOR_BELA)
	cmake_path(SET STK_DIR ${stk_SOURCE_DIR})
	cmake_path(SET STK_SOURCE_DIR ${STK_DIR}/src)
	cmake_path(SET STK_INCLUDE_DIR ${STK_DIR}/include)
	cmake_path(SET STK_RAWWAVES_DIR .)
	# get_filename_component(BELA_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

	file(GLOB STK_SOURCES
		${STK_SOURCE_DIR}/*.cpp
	)

	add_library(stk SHARED ${STK_SOURCES})
	target_compile_features(stk PRIVATE cxx_std_11)

	#target_compile_definitions(stk PUBLIC __LINUX_BELA__ __RTMIDI_DEBUG__)
	target_compile_definitions(stk PUBLIC __LINUX_BELA__ __STK_FLOAT__)
include(bela-lib)

target_link_directories(stk PUBLIC ${BELA_SYSROOT_PATH_LIST})
target_link_libraries(stk PUBLIC ${BELA_SYSROOT_LIB_LIST} pthread ${BELA} ${BELA_EXTRA})

target_include_directories(stk PUBLIC
	${CMAKE_BINARY_DIR}/external
	${STK_INCLUDE_DIR}
	PRIVATE
	${BELA_INCLUDE_DIR}
)
else ()
	if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
		message("${stk_SOURCE_DIR}")

		list(APPEND CMAKE_MODULE_PATH ${stk_SOURCE_DIR}/cmake)
		add_subdirectory(${stk_SOURCE_DIR} ${stk_BINARY_DIR})
		set(STK_INCLUDE_DIR ${stk_SOURCE_DIR}/include)
		set(STK_INCLUDE_DIR_LIST ${stk_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/external/  PARENT_SCOPE)
		cmake_path(SET STK_RAWWAVES_DIR ${stk_SOURCE_DIR})
		set(RTAUDIO_SAMPLE_RATE 48000.0)
	endif ()
endif ()

configure_file(${CMAKE_CURRENT_LIST_DIR}/stk-config.h.in ${CMAKE_BINARY_DIR}/external/stk-config.h @ONLY)