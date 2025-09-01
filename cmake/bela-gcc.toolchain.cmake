# get_filename_component(BELA_CMAKE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
list(APPEND CMAKE_MODULE_PATH ${BELA_CMAKE_DIR})

message("using xc-bela-toolchain")
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(triple arm-linux-gnueabihf)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
if(APPLE)
	find_program(CMAKE_C_COMPILER NAMES clang HINTS /opt/homebrew/opt/llvm/bin)
	find_program(CMAKE_CXX_COMPILER NAMES clang++ HINTS /opt/homebrew/opt/llvm/bin)
	find_program(CMAKE_ASM_COMPILER NAMES clang HINTS /opt/homebrew/opt/llvm/bin)
else()
    set(CMAKE_C_COMPILER /usr/bin/clang-10)
    set(CMAKE_CXX_COMPILER /usr/bin/clang++-10)
    # set(tools /usr/local/arm-linux-gnueabihf-binutils) ?
endif()

set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

if(DEFINED ENV{<XC_ROOT>})
    set(XC_ROOT ENV{<XC_ROOT>})
    message("using envvar XC_ROOT  - ${XC_ROOT}")
else()
    cmake_path(SET XC_ROOT NORMALIZE ${CMAKE_SOURCE_DIR})
    message("defaulting XC_ROOT to current")
endif()


cmake_path(SET XC_SYSROOT ${XC_ROOT}/sysroot)
message("using envvar XC_SYSROOT  - ${XC_SYSROOT}")
cmake_path(SET CMAKE_SYSROOT ${XC_SYSROOT})

list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ${XC_SYSROOT})

set(BELA_LINK_FLAGS "${BELA_LINK_FLAGS} -L${XC_SYSROOT}/usr/lib/gcc/arm-linux-gnueabihf/6.3.0  -B${XC_SYSROOT}/usr/lib/gcc/arm-linux-gnueabihf/6.3.0  -latomic")
set(BELA_LINK_FLAGS "${BELA_LINK_FLAGS} -Wl,-rpath-link,${XC_SYSROOT}/lib/arm-linux-gnueabihf")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${BELA_LINK_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${BELA_LINK_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${BELA_LINK_FLAGS}")

cmake_path(SET BELA_SYSROOT_PATH /usr/local/linaro/BelaSysroot)

list(APPEND BELA_SYSROOT_PATH_LIST 
    "${BELA_SYSROOT_PATH}/usr/lib" 
    "${BELA_SYSROOT_PATH}/usr/local/lib" 
    "${BELA_SYSROOT_PATH}/usr/xenomai/lib" 
    "${BELA_SYSROOT_PATH}/usr/lib/arm-linux-gnueabihf"
    )
list(APPEND BELA_SYSROOT_LIB_LIST cobalt asound seasocks prussdrv NE10)

cmake_path(SET BELA_LIB_DIR ${BELA_SYSROOT_PATH}/root/Bela/lib)