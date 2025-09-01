# Add the executable
add_executable(latelybass latelybass.cpp LBass.cpp LBass.h)

# target_compile_definitions(latelybass PUBLIC __MACOSX_CORE__)

target_link_libraries(latelybass PRIVATE stk)