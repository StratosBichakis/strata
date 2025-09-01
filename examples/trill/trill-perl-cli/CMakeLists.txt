# Add the executable
add_executable(trill-perl-cli trill-perl-cli.cpp ${BELA_PATH}/libraries/Trill/Trill.cpp)

target_include_directories(trill-perl-cli PRIVATE ${BELA_INCLUDE_DIR} ${BELA_PATH} ${BELA_SYSROOT_PATH}/usr/include ${PERL_INCLUDE_DIR})
target_link_directories(trill-perl-cli PRIVATE ${BELA_SYSROOT_PATH_LIST} ${PERL_LIB_DIR})
target_link_libraries(trill-perl-cli PRIVATE ${BELA_SYSROOT_LIB_LIST} ${BELA} ${BELA_EXTRA} ${LIB_PERL} ${LIB_M})