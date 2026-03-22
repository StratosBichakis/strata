if(CMAKE_CROSSCOMPILING)
    set(PERL_CORE_DIR ${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf/perl/5.24/CORE)
    set(PERL_INCLUDE_DIR ${PERL_CORE_DIR})
    set(PERL_LIB_DIR ${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf/)
    find_library(LIB_PERL NAMES perl HINTS ${PERL_LIB_DIR} REQUIRED)
    find_library(LIB_M NAMES m HINTS ${PERL_LIB_DIR} REQUIRED)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(PERL_CORE_DIR /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Perl/5.34/darwin-thread-multi-2level/CORE)
    set(PERL_INCLUDE_DIR ${PERL_CORE_DIR})
    set(PERL_LIB_DIR /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Perl/5.34/darwin-thread-multi-2level/CORE)
    find_library(LIB_PERL NAMES perl HINTS ${PERL_LIB_DIR} REQUIRED)
    find_library(LIB_M NAMES m REQUIRED)
endif()

function(create_perl_symlinks_simple SRC_DIR DEST_DIR)
    # 1. Validation
    if(NOT IS_DIRECTORY "${SRC_DIR}")
        message(WARNING "Perl source directory not found: ${SRC_DIR}")
        return()
    endif()

    # 2. Ensure the destination directory exists
    if(NOT EXISTS "${DEST_DIR}")
        file(MAKE_DIRECTORY "${DEST_DIR}")
    endif()

    # 3. Find files
    file(GLOB PERL_SOURCES "${SRC_DIR}/*.pl" "${SRC_DIR}/*.pm")

    # 4. Immediate execution
    foreach(SRC_PATH ${PERL_SOURCES})
        get_filename_component(FILE_NAME ${SRC_PATH} NAME)
        set(BIN_PATH "${DEST_DIR}/${FILE_NAME}")

        if(NOT EXISTS "${BIN_PATH}")
            message(STATUS "Creating symlink: ${FILE_NAME}")
            execute_process(
                    COMMAND ${CMAKE_COMMAND} -E create_symlink "${SRC_PATH}" "${BIN_PATH}"
            )
        endif()
    endforeach()
endfunction()