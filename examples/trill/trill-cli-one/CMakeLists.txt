# Add the executable
add_executable(trill-cli-one trill-cli-one.cpp ${BELA_PATH}/libraries/Trill/Trill.cpp)

target_include_directories(trill-cli-one PRIVATE ${BELA_INCLUDE_DIR} ${BELA_PATH} ${BELA_SYSROOT_PATH}/usr/include)
target_link_directories(trill-cli-one PRIVATE ${BELA_SYSROOT_PATH_LIST})
target_link_libraries(trill-cli-one PRIVATE ${BELA_SYSROOT_LIB_LIST} ${BELA} ${BELA_EXTRA})