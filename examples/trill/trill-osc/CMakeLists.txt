# Add the executable
add_executable(trill-osc trill-osc.cpp ${BELA_PATH}/libraries/Trill/Trill.cpp)

target_include_directories(trill-osc PRIVATE ${BELA_INCLUDE_DIR} ${BELA_PATH} ${BELA_SYSROOT_PATH}/usr/include)
target_link_directories(trill-osc PRIVATE ${BELA_SYSROOT_PATH_LIST})
target_link_libraries(trill-osc PRIVATE ${BELA_SYSROOT_LIB_LIST} ${BELA} ${BELA_EXTRA} oscpp)