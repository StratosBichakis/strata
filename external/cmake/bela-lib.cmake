list(APPEND BELA_SYSROOT_PATH_LIST
        "${XC_SYSROOT}/usr/lib"
        "${XC_SYSROOT}/usr/local/lib"
        "${XC_SYSROOT}/usr/xenomai/lib"
        "${XC_SYSROOT}/usr/lib/arm-linux-gnueabihf"
)
list(APPEND BELA_SYSROOT_LIB_LIST cobalt asound seasocks prussdrv NE10)

cmake_path(SET BELA_LIB_DIR ${XC_SYSROOT}/root/Bela/lib)

find_library(BELA NAMES bela PATHS ${BELA_LIB_DIR} REQUIRED)
find_library(BELA_EXTRA NAMES belaextra PATHS ${BELA_LIB_DIR} REQUIRED)
