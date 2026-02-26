# 1. Find all perl files in the current source directory
file(GLOB PERL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.pl" "${CMAKE_CURRENT_SOURCE_DIR}/*.pm")

# 2. Loop through each file and create a symlink in the binary directory
foreach(SRC_PATH ${PERL_SOURCES})
    # Get just the filename (e.g., "bang.pl")
    get_filename_component(FILE_NAME ${SRC_PATH} NAME)

    # Define the destination path in the build folder
    set(BIN_PATH "${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}")

    # Create the symlink only if it doesn't already exist
    if(NOT EXISTS "${BIN_PATH}")
        message(STATUS "Creating development symlink: ${FILE_NAME}")
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E create_symlink "${SRC_PATH}" "${BIN_PATH}"
        )
    endif()
endforeach()
# Add the executable
add_executable(bang-perl-cli bang-perl-cli.cpp)

target_include_directories(bang-perl-cli PRIVATE ${PERL_INCLUDE_DIR})
target_link_directories(bang-perl-cli PRIVATE ${PERL_LIB_DIR})
target_link_libraries(bang-perl-cli PRIVATE ${LIB_PERL} ${LIB_M} pthread)

#configure_file(bang.pl ${CMAKE_CURRENT_BINARY_DIR}/bang.pl COPYONLY)
