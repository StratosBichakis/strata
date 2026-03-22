include(FetchContent)

FetchContent_Declare(
	oscpp
	GIT_REPOSITORY https://github.com/kaoskorobase/oscpp.git
	GIT_TAG        805365c3b7b7a5c819866040ab434f011dfcfbf9 # oscpp-include
	GIT_SHALLOW TRUE
	GIT_SUBMODULES ""
	SOURCE_SUBDIR     pathThatDoesNotExist
)

FetchContent_MakeAvailable(oscpp)

add_library(oscpp INTERFACE)
target_include_directories(oscpp INTERFACE "${oscpp_SOURCE_DIR}/include")
