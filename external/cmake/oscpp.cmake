include(FetchContent)
FetchContent_Declare(
	oscpp
	GIT_REPOSITORY https://github.com/kaoskorobase/oscpp.git
	GIT_TAG        master # oscpp-include
	GIT_SHALLOW TRUE
	SOURCE_SUBDIR     pathThatDoesNotExist
)
FetchContent_MakeAvailable(oscpp)
cmake_path(SET OSCPP_DIR ${oscpp_SOURCE_DIR}/include)

add_library(oscpp INTERFACE)

target_include_directories(oscpp INTERFACE ${OSCPP_DIR})
