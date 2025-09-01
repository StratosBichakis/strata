# Add the executable
add_executable(latelybass latelybass.cpp LBass.cpp LBass.h)

# target_compile_definitions(latelybass PUBLIC __MACOSX_CORE__)

target_include_directories(latelybass PRIVATE ${BELA_INCLUDE_DIR} ${BELA_SYSROOT_PATH}/usr/include ${CMAKE_BINARY_DIR}/external)
target_link_directories(latelybass PRIVATE ${BELA_SYSROOT_PATH_LIST})
target_link_libraries(latelybass PRIVATE ${BELA_SYSROOT_LIB_LIST} ${BELA} ${BELA_EXTRA} stk)