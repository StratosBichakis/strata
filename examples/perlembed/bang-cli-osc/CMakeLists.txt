create_perl_symlinks_simple("${CMAKE_CURRENT_SOURCE_DIR}/scripts/" "${CMAKE_CURRENT_BINARY_DIR}")
add_executable(bang-perl-cli-osc bang-perl-cli-osc.cpp)

target_include_directories(bang-perl-cli-osc PRIVATE ${PERL_INCLUDE_DIR})
target_link_directories(bang-perl-cli-osc PRIVATE ${PERL_LIB_DIR})
target_link_libraries(bang-perl-cli-osc PRIVATE ${LIB_PERL} ${LIB_M} oscpp pthread)

add_bela_deploy_target(bang-perl-cli-osc)
