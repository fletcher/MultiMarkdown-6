# Settings for compiling for djgpp 32-bit machines

set (IS_CROSSCOMPILING "YES")
set (IS_32_BIT "32-")
set (CMAKE_C_FLAGS "-DDJGPP")

set (CMAKE_C_COMPILER i586-pc-msdosdjgpp-gcc)
set (CMAKE_CXX_COMPILER i586-pc-msdosdjgpp-g++)

set (CMAKE_FIND_ROOT_PATH /usr/bin)
