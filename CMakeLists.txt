cmake_minimum_required(VERSION 3.30)

project(strata LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/external/cmake)

include(perl)
add_subdirectory(external)

add_subdirectory(examples/minimal)
add_subdirectory(examples/file-player)
add_subdirectory(examples/latelybass)

if(BUILD_FOR_BELA)
add_subdirectory(examples/trill/trill-cli-one)
add_subdirectory(examples/trill/trill-cli)
add_subdirectory(examples/trill/trill-osc)
add_subdirectory(examples/trill/trill-perl-cli)
add_subdirectory(examples/trill/trill-perl-cli-one)
endif ()

#add_subdirectory(examples/bead-controller)
add_subdirectory(examples/perlembed)
