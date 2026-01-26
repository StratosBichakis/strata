list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(BUILD_FOR_BELA)
    include(bela)
endif()

include(stk)
include(oscpp)