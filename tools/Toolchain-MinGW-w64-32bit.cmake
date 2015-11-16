# Settings for compiling for Windows 32-bit machines using MinGW-w64

set (IS_CROSSCOMPILING "YES")
set (IS_32_BIT "32-")

set (CMAKE_SYSTEM_NAME Windows)

set (CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set (CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set (CMAKE_RC_COMPILER i686-w64-mingw32-windres)

set (CMAKE_FIND_ROOT_PATH /usr/bin)
