# Settings for compiling for djgpp 32-bit machines

set (IS_CROSSCOMPILING "YES")
set (IS_32_BIT "32-")

set (CMAKE_SYSTEM_NAME DJGPP)

set (CMAKE_C_COMPILER i586-pc-msdosdjgpp-gcc)
set (CMAKE_CXX_COMPILER i586-pc-msdosdjgpp-g++)
set (CMAKE_RC_COMPILER i586-pc-msdosdjgpp-windres)

set (CMAKE_FIND_ROOT_PATH /usr/bin)
