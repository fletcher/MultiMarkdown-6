cmake_minimum_required (VERSION 2.6)


# ==================
# Define Our Project
# ==================

set (My_Project_Title "MultiMarkdown")
set (My_Project_Description "Lightweight markup processor to produce HTML, LaTeX, and more.")
set (My_Project_Author "Fletcher T. Penney")
set (My_Project_Revised_Date "2017-07-06")
set (My_Project_Version_Major 6)
set (My_Project_Version_Minor 1)
set (My_Project_Version_Patch 0)

set (My_Project_Version "${My_Project_Version_Major}.${My_Project_Version_Minor}.${My_Project_Version_Patch}")

set (My_Project_Copyright_Date "2016 - 2017")
set (My_Project_Copyright "Copyright © ${My_Project_Copyright_Date} ${My_Project_Author}.")

string(TOUPPER ${My_Project_Title} My_Project_Title_Caps  )
string(REGEX REPLACE " " "_" My_Project_Title_Caps ${My_Project_Title_Caps} )

project (${My_Project_Title})


# Enable this if you want to be warned about struct alignment
# add_definitions ("-Wpadded")


# =========================
# Build Submodules (if any)
# =========================

# add_subdirectory(submodules/foo)


# =================
# Configure Project
# =================

# Search for included files here
include_directories( ${PROJECT_SOURCE_DIR}/Sources/libMultiMarkdown)
include_directories( ${PROJECT_SOURCE_DIR}/Sources/libMultiMarkdown/include)
include_directories( ${PROJECT_SOURCE_DIR}/Sources/multimarkdown)
include_directories( ${PROJECT_SOURCE_DIR}/test )
include_directories(${PROJECT_BINARY_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# using Clang
	# Default is 256 -- needed for localization hash function
	add_definitions("-fbracket-depth=264")	
else()
	add_definitions("-std=c99")
endif()

set(CMAKE_SUPPRESS_REGENERATION 1)


# =================
# Macro Definitions
# =================

MACRO(ADD_PUBLIC_HEADER target filename)
	# Add filename to public_header_files list, flag it as 
	# public header for libraries and OS X Frameworks

	# This will work for creating one library/framework with public headers
	# per project. If you need more than one, you will need to customize
	# the workflow as appropriate, since there is only one 
	# public_header_files list.

	# This should only be run once per target
	# SET_TARGET_PROPERTIES(${target} PROPERTIES PUBLIC_HEADER ${filename})

	LIST(APPEND public_header_files ${filename})

	SET_SOURCE_FILES_PROPERTIES(
		${filename}
		PROPERTIES
		MACOSX_PACKAGE_LOCATION
		include/\${PRODUCT_NAME}
	)

	# Set Xcode project to configure public header location to allow
	# use when this project is used in another workspace.
	# NOTE: You must manually add a "Headers" build phase and add
	# the desired public headers to that list for Xcode to use them.
	#
	# TODO: If anyone knows how to automate that in Cmake, I would be very
	# grateful!!

	SET_TARGET_PROPERTIES(${target} PROPERTIES
		XCODE_ATTRIBUTE_PUBLIC_HEADERS_FOLDER_PATH
		"include/$(TARGET_NAME)"
	)

	SET_TARGET_PROPERTIES(${target} PROPERTIES
		XCODE_ATTRIBUTE_PRIVATE_HEADERS_FOLDER_PATH
		"$(PUBLIC_HEADERS_FOLDER_PATH)/Private"
	)
	
	# Set Xcode target to include settings for OS X and iOS
	
	SET_TARGET_PROPERTIES(${target} PROPERTIES
		XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS
		"macos iphonesimulator iphoneos"
	)

	SET_TARGET_PROPERTIES(${target} PROPERTIES
		XCODE_ATTRIBUTE_VALID_ARCHITECTURES
		"x86_64 i386 armv6 armv7 armv7s arm64"
	)

ENDMACRO(ADD_PUBLIC_HEADER)


# The target should be an OS X Bundle with a PList

MACRO(MAKE_TARGET_BUNDLE targetname)

	set_target_properties(
		${targetname}
		PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST
		${PROJECT_SOURCE_DIR}/templates/plist.in
	)

ENDMACRO(MAKE_TARGET_BUNDLE)


MACRO(ADD_LINKED_FRAMEWORK frame)

	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGES} -framework ${frame}")

ENDMACRO(ADD_LINKED_FRAMEWORK)


# ======================
# Process Template Files
# ======================

file(READ ${PROJECT_SOURCE_DIR}/LICENSE.txt My_Project_License)

string(REGEX REPLACE "\n" "\n\t" My_Project_License_Indented ${My_Project_License})

string(REGEX REPLACE "\"" "\\\\\"" My_Project_License_Escaped ${My_Project_License_Indented})

string(REGEX REPLACE "\n" "\\\\n\"\\\\\n\"" My_Project_License_Literal ${My_Project_License_Escaped})


configure_file (
	"${PROJECT_SOURCE_DIR}/templates/template.c.in"
	"${PROJECT_BINARY_DIR}/template.c"
)

configure_file (
	"${PROJECT_SOURCE_DIR}/templates/template.h.in"
	"${PROJECT_BINARY_DIR}/template.h"
)

# Update the project README, to pull in new version #, etc.
configure_file (
	"${PROJECT_SOURCE_DIR}/templates/README.md.in"
	"${CMAKE_CURRENT_LIST_DIR}/README.md"
)

configure_file (
	"${PROJECT_SOURCE_DIR}/templates/version.h.in"
	"${PROJECT_BINARY_DIR}/version.h"
)


# ============
# Source Files
# ============

# src_files are the primary files, and will be included in doxygen documentation
set(src_files
	Sources/libMultiMarkdown/aho-corasick.c
	Sources/libMultiMarkdown/beamer.c
	Sources/libMultiMarkdown/char.c
	Sources/libMultiMarkdown/critic_markup.c
	Sources/libMultiMarkdown/d_string.c
	Sources/libMultiMarkdown/epub.c
	Sources/libMultiMarkdown/fodt.c
	Sources/libMultiMarkdown/html.c
	Sources/libMultiMarkdown/latex.c
	Sources/libMultiMarkdown/lexer.c
	Sources/libMultiMarkdown/memoir.c
	Sources/libMultiMarkdown/miniz.c
	Sources/libMultiMarkdown/mmd.c
	Sources/libMultiMarkdown/object_pool.c
	Sources/libMultiMarkdown/opendocument.c
	Sources/libMultiMarkdown/opendocument-content.c
	Sources/libMultiMarkdown/parser.c
	Sources/libMultiMarkdown/rng.c
	Sources/libMultiMarkdown/scanners.c
	Sources/libMultiMarkdown/stack.c
	Sources/libMultiMarkdown/textbundle.c
	Sources/libMultiMarkdown/token.c
	Sources/libMultiMarkdown/token_pairs.c
	Sources/libMultiMarkdown/transclude.c
	Sources/libMultiMarkdown/uuid.c
	Sources/libMultiMarkdown/writer.c
	Sources/libMultiMarkdown/zip.c
)

# Primary header files, also for doxygen documentation
set(header_files
	Sources/libMultiMarkdown/aho-corasick.h
	Sources/libMultiMarkdown/beamer.h
	Sources/libMultiMarkdown/char.h
	Sources/libMultiMarkdown/critic_markup.h
	Sources/libMultiMarkdown/include/d_string.h
	Sources/libMultiMarkdown/epub.h
	Sources/libMultiMarkdown/fodt.h
	Sources/libMultiMarkdown/html.h
	Sources/libMultiMarkdown/latex.h
	Sources/libMultiMarkdown/lexer.h
	Sources/libMultiMarkdown/include/libMultiMarkdown.h
	Sources/libMultiMarkdown/memoir.h
	Sources/libMultiMarkdown/miniz.h
	Sources/libMultiMarkdown/mmd.h
	Sources/libMultiMarkdown/object_pool.h
	Sources/libMultiMarkdown/opendocument.h
	Sources/libMultiMarkdown/opendocument-content.h
	Sources/libMultiMarkdown/scanners.h
	Sources/libMultiMarkdown/stack.h
	Sources/libMultiMarkdown/textbundle.c
	Sources/libMultiMarkdown/include/token.h
	Sources/libMultiMarkdown/token_pairs.h
	Sources/libMultiMarkdown/transclude.h
	Sources/libMultiMarkdown/uthash.h
	Sources/libMultiMarkdown/uuid.h
	Sources/libMultiMarkdown/writer.h
	Sources/libMultiMarkdown/zip.h
)

if (WIN32)
	set (scripts
		scripts/mmd.bat
		scripts/mmd2all.bat
		scripts/mmd2epub.bat
		scripts/mmd2fodt.bat
		scripts/mmd2odt.bat
		scripts/mmd2opml.bat
		scripts/mmd2tex.bat
		scripts/markdown.bat
	)
else()
	set (scripts
		scripts/mmd
		scripts/mmd2all
		scripts/mmd2epub
		scripts/mmd2fodt
		scripts/mmd2odt
		scripts/mmd2opml
		scripts/mmd2pdf
		scripts/mmd2tex
		scripts/markdown
	)
endif (WIN32)

set (latex
	texmf/tex/latex/mmd6/README.md

	texmf/tex/latex/mmd6/article/mmd6-article-begin.tex
	texmf/tex/latex/mmd6/article/mmd6-article-footer.tex
	texmf/tex/latex/mmd6/article/mmd6-article-leader.tex

	texmf/tex/latex/mmd6/letterhead/mmd-envelope.sty
	texmf/tex/latex/mmd6/letterhead/mmd-letterhead.sty
	texmf/tex/latex/mmd6/letterhead/mmd6-letterhead-begin.tex
	texmf/tex/latex/mmd6/letterhead/mmd6-letterhead-footer.tex
	texmf/tex/latex/mmd6/letterhead/mmd6-letterhead-leader.tex

	texmf/tex/latex/mmd6/shared/mmd6-criticmarkup.tex
	texmf/tex/latex/mmd6/shared/mmd6-default-metadata.tex
	texmf/tex/latex/mmd6/shared/mmd6-memoir-footer.tex
	texmf/tex/latex/mmd6/shared/mmd6-memoir-layout-8.5x11.tex
	texmf/tex/latex/mmd6/shared/mmd6-memoir-packages.tex
	texmf/tex/latex/mmd6/shared/mmd6-memoir-setup.tex
	texmf/tex/latex/mmd6/shared/mmd6-title.tex
	texmf/tex/latex/mmd6/shared/mmd6-tufte-footer.tex

	texmf/tex/latex/mmd6/tufte-book/mmd6-tufte-book-begin.tex
	texmf/tex/latex/mmd6/tufte-book/mmd6-tufte-book-footer.tex
	texmf/tex/latex/mmd6/tufte-book/mmd6-tufte-book-leader.tex
	texmf/tex/latex/mmd6/tufte-handout/mmd6-tufte-handout-begin.tex
	texmf/tex/latex/mmd6/tufte-handout/mmd6-tufte-handout-footer.tex
	texmf/tex/latex/mmd6/tufte-handout/mmd6-tufte-handout-leader.tex
)

# Public headers, will be installed in 'include'
# Do not manually add files here, use the ADD_PUBLIC_HEADER() macro
set(public_header_files
)

# Utility source files will not be included in doxygen
set(src_utility_files
)

set(header_utility_files
	${PROJECT_BINARY_DIR}/version.h
)

# Generate doxygen configuration file
string(REPLACE ";" " " doxygen_src_files "${src_files}" )
string(REPLACE ";" " " doxygen_header_files "${header_files}" )

configure_file (
	"${PROJECT_SOURCE_DIR}/templates/doxygen.conf.in"
	"${PROJECT_BINARY_DIR}/doxygen.conf"
)


# ===========================================
# Build Test Suite with CuTest (unit testing)
# ===========================================

# from http://stackoverflow.com/questions/25199677/how-to-detect-if-current-scope-has-a-parent-in-cmake
get_directory_property(hasParent PARENT_DIRECTORY)

if(hasParent)
	# Don't create "run_tests" when we're a sub-project for something else
else()
	set(test_files
		test/CuTest.c
		test/CuTest.h
		${PROJECT_BINARY_DIR}/AllTests.c
	)

	if (DEFINED TEST)
		add_definitions(-DTEST)

		add_executable(run_tests
			${test_files}
			${src_files}
			${header_files}
			${src_utility_files}
			${header_utility_files}
		)

		# Process source files to look for tests to run
		add_custom_command (
			OUTPUT ${PROJECT_BINARY_DIR}/AllTests.c
			COMMAND sh ${PROJECT_SOURCE_DIR}/test/make-tests.sh ${PROJECT_SOURCE_DIR}/Sources/libMultiMarkdown/*.c > ${PROJECT_BINARY_DIR}/AllTests.c
		)

		enable_testing()

		add_test( test ${PROJECT_BINARY_DIR}/run_tests)

		# valgrind memory testing
	#	find_program (MEMORYCHECK_COMMAND valgrind)
	#	SET (MEMORYCHECK_COMMAND_OPTIONS --leak-check=full --error-exitcode=1)
	#
	#	add_test( memory_test ${MEMORYCHECK_COMMAND} ${MEMORYCHECK_COMMAND_OPTIONS} ${PROJECT_BINARY_DIR}/run_tests)

	endif()
endif()


# =======================
# Configure for Target OS
# =======================

# OS X Builds
if (APPLE)

	# Configure backwards-compatible support (if your project allows it)
	SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.4" CACHE STRING "Deployment target for OSX" FORCE)

	# Compile for x86_64 and i386.  ppc no longer supported
	if(CMAKE_BUILD_TYPE MATCHES "Release")
		SET (CMAKE_OSX_ARCHITECTURES x86_64;i386)
	endif(CMAKE_BUILD_TYPE MATCHES "Release")

	# Use PackageMaker for installers?
	if (DEFINED ZIP)
		set (CPACK_GENERATOR ZIP)
	else (DEFINED ZIP)
		set (CPACK_GENERATOR PackageMaker)
	endif (DEFINED ZIP)

endif (APPLE)

# Windows Builds
if (WIN32)

	# Use NSIS to generate installers?
	if (DEFINED ZIP)
		set (CPACK_GENERATOR ZIP)
	else (DEFINED ZIP)
		set (CPACK_GENERATOR NSIS)
	endif (DEFINED ZIP)

# Linux Builds (not cross-compiling for Windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	# Create zip archive
	set (CPACK_GENERATOR ZIP)

	# Statically link libraries -- might make the binary slightly more
	# compatible across Linux distributions, for example
	#
	# You may wish to disable this.
	#
	
	# set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	# set (BUILD_SHARED_LIBRARIES OFF)
	# set (CMAKE_EXE_LINKER_FLAGS "-static")

	# Some libraries need to be linked on some Linux builds
	if (DEFINED TEST)
		# target_link_libraries(run_tests m)
	endif (DEFINED TEST)

endif (WIN32)


# ==============
# Define targets
# ==============

# Is libcurl available?
find_package(CURL)
if (CURL_FOUND)
	add_definitions(-DUSE_CURL)
	message (STATUS "libcurl found")
else ()
	message (STATUS "libcurl not found")
endif (CURL_FOUND)	

# Create a library?
if (NOT DEFINED TEST)
	add_library(libMultiMarkdown STATIC
		${src_files}
		${src_utility_files}
		${header_files}
		${header_utility_files}
	)
else ()
	add_library(libMultiMarkdown STATIC
		${src_files}
		${src_utility_files}
		${header_files}
		${header_utility_files}
		${test_files}
	)
endif ()

if (CURL_FOUND) 
	target_link_libraries(libMultiMarkdown curl)
endif (CURL_FOUND)	

# Create a shared library if defined
if (DEFINED SHAREDBUILD)

	add_library(libMultiMarkdownShared SHARED
		${src_files}
		${src_utility_files}
		${header_files}
		${header_utility_files}
	)

	# Shared libraries require position independent code
	SET_TARGET_PROPERTIES(libMultiMarkdownShared PROPERTIES POSITION_INDEPENDENT_CODE 1)

	# Remove the extra "lib" from "liblibMultiMarkdown"
	SET_TARGET_PROPERTIES(libMultiMarkdownShared PROPERTIES PREFIX "")

	# Remove "Shared" from library name
	SET_TARGET_PROPERTIES(libMultiMarkdownShared PROPERTIES OUTPUT_NAME libMultiMarkdown)

	if (CURL_FOUND) 
		target_link_libraries(libMultiMarkdownShared curl)
	endif (CURL_FOUND)	

endif (DEFINED SHAREDBUILD)

# remove the extra "lib" from "liblibFOO"
SET_TARGET_PROPERTIES(libMultiMarkdown PROPERTIES
	PREFIX ""
	FRAMEWORK TRUE
	FRAMEWORK_VERSION A
	MACOSX_FRAMEWORK_IDENTIFIER net.multimarkdown.mmd6
)

ADD_PUBLIC_HEADER(libMultiMarkdown Sources/libMultiMarkdown/include/libMultiMarkdown.h)
ADD_PUBLIC_HEADER(libMultiMarkdown Sources/libMultiMarkdown/include/d_string.h)
ADD_PUBLIC_HEADER(libMultiMarkdown Sources/libMultiMarkdown/include/token.h)
ADD_PUBLIC_HEADER(libMultiMarkdown Sources/libMultiMarkdown/include/version.h)

# Create a command-line app?
# if (NOT DEFINED TEST)
	add_executable(multimarkdown
		Sources/libMultiMarkdown/d_string.c
		Sources/multimarkdown/main.c
		Sources/multimarkdown/argtable3.c
	)
# 
#	Link the library to the app?
	target_link_libraries(multimarkdown libMultiMarkdown)
# endif()

# Xcode settings for fat binaries
set_target_properties(libMultiMarkdown PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
set_target_properties(multimarkdown PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")

# Use C11
set_property(TARGET libMultiMarkdown PROPERTY C_STANDARD 11)
set_property(TARGET multimarkdown PROPERTY C_STANDARD 11)


# ==========================
# Build Installer with CPack
# ==========================

# You're largely on your own here

# install (FILES ${CMAKE_CURRENT_LIST_DIR}/README.md ${PROJECT_SOURCE_DIR}/LICENSE.txt
# 	DESTINATION .
# )

# Use something like this to install public header files (after adding them
# with the ADD_PUBLIC_HEADER() macro)

# install (FILES ${public_header_files} DESTINATION local/include/libFoo) 

# Install MMD binary
install (TARGETS multimarkdown
	DESTINATION bin
	COMPONENT application
)
set (CPACK_COMPONENT_APPLICATION_DISPLAY_NAME "MultiMarkdown")
set (CPACK_COMPONENT_APPLICATION_DESCRIPTION "Install the actual `multimarkdown` program.")


# Install utility scripts
install (FILES ${scripts}
	DESTINATION bin
	COMPONENT scripts
	PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
set (CPACK_COMPONENT_SCRIPTS_DISPLAY_NAME "Convenience scripts")
set (CPACK_COMPONENT_SCRIPTS_DESCRIPTION "Install convenience scripts for common MultiMarkdown shortcuts, e.g. `mmd`, `mmd2tex`, etc.")

# Install Shared Library?
if (DEFINED SHAREDBUILD)
	install (TARGETS libMultiMarkdownShared
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		COMPONENT sharedlib
	)

	install (FILES ${public_header_files}
		DESTINATION include/multimarkdown
		COMPONENT sharedlib
	)

	set (CPACK_COMPONENT_SHAREDLIB_DISPLAY_NAME "Shared Library")
	set (CPACK_COMPONENT_SHAREDLIB_DESCRIPTION "MultiMarkdown shared library for use with third party software.")

	set (CPACK_COMPONENTS_ALL application scripts sharedlib)

	set (CPACK_COMPONENT_SHAREDLIB_GROUP "MultiMarkdown")
endif (DEFINED SHAREDBUILD)

# Install LaTeX support files
install (FILES ${latex}
	DESTINATION share/texmf/tex/latex/mmd6
	COMPONENT latex
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
)
set (CPACK_COMPONENT_LATEX_DISPLAY_NAME "LaTeX support files")
set (CPACK_COMPONENT_LATEX_DESCRIPTION "Install configuration files for MultiMarkdown-based LaTex configurations.")


# Which groups should be installed?
set (CPACK_COMPONENTS_ALL application scripts latex)

set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "${My_Project_Description}")
set (CPACK_PACKAGE_VENDOR "fletcherpenney.net")
set (CPACK_PACKAGE_VERSION "${My_Project_Version_Major}.${My_Project_Version_Minor}.${My_Project_Version_Patch}")
set (CPACK_PACKAGE_VERSION_MAJOR "${My_Project_Version_Major}")
set (CPACK_PACKAGE_VERSION_MINOR "${My_Project_Version_Minor}")
set (CPACK_PACKAGE_VERSION_PATCH "My_Project_Version_Patch")

set (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")

if (APPLE)
	set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-Mac-${CPACK_PACKAGE_VERSION}")
else (APPLE)
	set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CMAKE_SYSTEM_NAME}-${CPACK_PACKAGE_VERSION}")
endif (APPLE)

# Use some default install locations (if appropriate)
set(CPACK_SET_DESTDIR true)
if (WIN32)
	set (CPACK_INSTALL_PREFIX "")
	set (CPACK_NSIS_MODIFY_PATH ON)
else (WIN32)
	set (CPACK_INSTALL_PREFIX /usr/local)
endif (WIN32)

set (CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT})

set (CPACK_PACKAGE_CONTACT "fletcher@fletcherpenney.net")

include (CPack)


# ======================
# Integration Test Suite
# ======================

enable_testing()

function(ADD_MMD_TEST NAME FLAGS FOLDER EXTENSION)
	add_test ( ${NAME}
		${PROJECT_SOURCE_DIR}/tests/MarkdownTest.pl
		--Script="${CMAKE_CURRENT_BINARY_DIR}/multimarkdown"
		--testdir=${PROJECT_SOURCE_DIR}/tests/${FOLDER}
		"--Flags=${FLAGS}"
		--ext=${EXTENSION}
	)

endfunction(ADD_MMD_TEST)


# MMD 6
ADD_MMD_TEST(mmd-6 "" MMD6Tests html)

ADD_MMD_TEST(mmd-6-compat "-c" MMD6Tests htmlc)

ADD_MMD_TEST(mmd-6-latex "-t latex" MMD6Tests tex)

ADD_MMD_TEST(mmd-6-odf "-t fodt" MMD6Tests fodt)

ADD_MMD_TEST(mmd-6-critic-accept "-a" CriticMarkup htmla)

ADD_MMD_TEST(mmd-6-critic-reject "-r" CriticMarkup htmlr)

ADD_MMD_TEST(pathologic-compat "-c" ../build html)

ADD_MMD_TEST(pathologic "" ../build html)
