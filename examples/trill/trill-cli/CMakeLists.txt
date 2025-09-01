# Add the executable
add_executable(trill-cli trill-cli.cpp ${BELA_PATH}/libraries/Trill/Trill.cpp)

target_compile_features(trill-cli PUBLIC cxx_std_14)

target_include_directories(trill-cli PRIVATE ${BELA_INCLUDE_DIR} ${BELA_PATH} ${BELA_SYSROOT_PATH}/usr/include)
target_link_directories(trill-cli PRIVATE ${BELA_SYSROOT_PATH_LIST})
target_link_libraries(trill-cli PRIVATE ${BELA_SYSROOT_LIB_LIST} ${BELA} ${BELA_EXTRA})