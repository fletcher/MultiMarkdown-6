# Settings for compiling for Windows 64-bit machines using MinGW-w64

set (IS_CROSSCOMPILING "YES")

set (CMAKE_SYSTEM_NAME Windows)

set (CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set (CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set (CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set (CMAKE_FIND_ROOT_PATH /usr/bin)
