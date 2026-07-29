cmake_minimum_required (VERSION 3.5)

# Quick file to build lemon parser generator

add_executable(lemon
	lemon.c
)

configure_file(
	lempar.c ${PROJECT_BINARY_DIR}/lempar.c COPYONLY
)
