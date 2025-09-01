include(FetchContent)
FetchContent_Populate(
	oscpp
	GIT_REPOSITORY https://github.com/kaoskorobase/oscpp.git
	GIT_TAG        master # oscpp-include
)

cmake_path(SET OSCPP_DIR ${oscpp_SOURCE_DIR}/include)

add_library(oscpp INTERFACE)

target_include_directories(oscpp INTERFACE ${OSCPP_DIR})
