create_perl_symlinks_simple("${CMAKE_CURRENT_SOURCE_DIR}/scripts" "${CMAKE_CURRENT_BINARY_DIR}")
add_executable(bang-perl-cli bang-perl-cli.cpp)

target_include_directories(bang-perl-cli PRIVATE ${PERL_INCLUDE_DIR})
target_link_directories(bang-perl-cli PRIVATE ${PERL_LIB_DIR})
target_link_libraries(bang-perl-cli PRIVATE ${LIB_PERL} ${LIB_M} pthread)

add_bela_deploy_target(bang-perl-cli)

#configure_file(bang.pl ${CMAKE_CURRENT_BINARY_DIR}/bang.pl COPYONLY)
