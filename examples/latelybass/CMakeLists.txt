# Add the executable
add_executable(latelybass latelybass.cpp LBass.cpp LBass.h)

target_link_libraries(latelybass PRIVATE stk)
