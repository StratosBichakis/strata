include(FetchContent)
FetchContent_Declare(
  	stk
  	GIT_REPOSITORY https://github.com/StratosBichakis/stk.git
  	GIT_TAG        e76073f926ec2ffec90124ef51222246831cd4ea # bela-rtAudio branch
	GIT_SHALLOW TRUE
	SOURCE_SUBDIR     pathThatDoesNotExist
)
FetchContent_MakeAvailable(stk)

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
	target_compile_definitions(stk PUBLIC __LINUX_BELA__)
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
		message("^")
		set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
		list(APPEND CMAKE_MODULE_PATH ${stk_SOURCE_DIR}/cmake)
		add_subdirectory(${stk_SOURCE_DIR} ${stk_BINARY_DIR})
		set(STK_INCLUDE_DIR ${stk_SOURCE_DIR}/include)
		set(STK_INCLUDE_DIR_LIST ${stk_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/external/  PARENT_SCOPE)
		cmake_path(SET STK_RAWWAVES_DIR ${stk_SOURCE_DIR})
#		find_library(stk-lib NAMES stk PATHS ${stk_BINARY_DIR} REQUIRED)
#		find_library(Foundation-lib Foundation REQUIRED)
#		find_library(AudioToolbox-lib AudioToolbox REQUIRED)
#		find_library(pthread-lib pthread REQUIRED)

#		target_compile_definitions(latelybass PUBLIC __MACOSX_CORE__)

#		target_link_libraries(latelybass PRIVATE ${stk-lib} ${Foundation-lib} ${AudioToolbox-lib} ${pthread-lib})

		# Include the STK headers
#		target_include_directories(latelybass PRIVATE ${STK_INCLUDE_DIR})
	endif ()
endif ()

configure_file(${CMAKE_CURRENT_LIST_DIR}/stk-config.h.in ${CMAKE_BINARY_DIR}/external/stk-config.h @ONLY)