if((NOT BELA_PATH) AND (DEFINED ENV{BELA_PATH}))
    cmake_path(SET BELA_PATH $ENV{BELA_PATH})
    message(STATUS "ENV BELA_PATH specified, using BELA_PATH: ${BELA_PATH}")
endif()

if(NOT BELA_PATH)
    include(FetchContent)
    FetchContent_Populate(
        bela
        GIT_REPOSITORY https://github.com/BelaPlatform/Bela.git
        GIT_TAG        master 
    )
    cmake_path(SET BELA_PATH "${bela_SOURCE_DIR}")
endif()

cmake_path(SET BELA_CORE_DIR ${BELA_PATH}/core)
cmake_path(SET BELA_INCLUDE_DIR ${BELA_PATH}/include)

set(BELA_PATH ${BELA_PATH} PARENT_SCOPE)
set(BELA_CORE_DIR ${BELA_CORE_DIR} PARENT_SCOPE)
set(BELA_INCLUDE_DIR ${BELA_INCLUDE_DIR} PARENT_SCOPE)