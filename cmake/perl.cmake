set(PERL_CORE_DIR ${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf/perl/5.24/CORE)
set(PERL_INCLUDE_DIR ${PERL_CORE_DIR})
set(PERL_LIB_DIR ${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf/)

find_library(LIB_PERL NAMES perl PATHS ${PERL_LIB_DIR} REQUIRED)
find_library(LIB_M NAMES m HINTS ${PERL_LIB_DIR} REQUIRED)