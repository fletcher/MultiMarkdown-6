#
#	Boilerplate CMake configuration for a cross-platform library, 
#	including configuration for a static framework on Xcode/macOS/iOS
#
#	Designed by Fletcher T. Penney
#	See the LICENSE file for copyright and licensing information.
#

cmake_minimum_required(VERSION 2.9)


# ===================
# Customize This Part
# ===================

# Project Description

set (My_Project_Title "libMultiMarkdown")
set (My_Project_Description "Lightweight markup processor to produce HTML, LaTeX, and more.")
set (My_Project_Author "Fletcher T. Penney")
set (My_Project_Revised_Date "2020-10-28")
set (My_Project_Version_Major 6)
set (My_Project_Version_Minor 6)
set (My_Project_Version_Patch 0)

set (My_Project_Copyright_Date "2016 - 2020")

set (My_Project_Identifier "net.fletcherpenney.multimarkdown")

string(TIMESTAMP My_Build_Version "%Y.%m.%d.%H.%M")


# Search for included files here
include_directories( ${PROJECT_SOURCE_DIR}/src)


# Source files and headers

set(src_files
	src/aho-corasick.c
	src/beamer.c
	src/char.c
	src/critic_markup.c
	src/d_string.c
	src/epub.c
	src/file.c
	src/html.c
	src/itmz.c
	src/itmz-lexer.c
	src/itmz-parser.c
	src/itmz-reader.c
	src/latex.c
	src/lexer.c
	src/memoir.c
	src/miniz.c
	src/mmd.c
	src/object_pool.c
	src/opendocument.c
	src/opendocument-content.c
	src/opml.c
	src/opml-lexer.c
	src/opml-parser.c
	src/opml-reader.c
	src/parser.c
	src/rng.c
	src/scanners.c
	src/stack.c
	src/textbundle.c
	src/token.c
	src/token_pairs.c
	src/transclude.c
	src/uuid.c
	src/xml.c
	src/writer.c
	src/zip.c
)

set(public_headers
	src/libMultiMarkdown.h
	src/d_string.h
	src/token.h
)

set(private_headers
	src/aho-corasick.h
	src/beamer.h
	src/char.h
	src/critic_markup.h
	src/epub.h
	src/file.h
	src/html.h
	src/itmz.h
	src/itmz-lexer.h
	src/itmz-parser.h
	src/itmz-reader.h
	src/latex.h
	src/lexer.h
	src/memoir.h
	src/miniz.h
	src/mmd.h
	src/object_pool.h
	src/opendocument.h
	src/opendocument-content.h
	src/opml.h
	src/opml-lexer.h
	src/opml-parser.h
	src/opml-reader.h
	src/scanners.h
	src/stack.h
	src/textbundle.c
	src/token_pairs.h
	src/transclude.h
	src/uthash.h
	src/uuid.h
	src/xml.h
	src/writer.h
	src/zip.h
)

set(resources
)

set(main_storyboard
)


# Include other CMake projects?
# add_subdirectory(submodules/foo)

# Libraries for linking
# on macOS can link to frameworks using: "-framework QuartzCore", for example
set(libraries_to_link
)

# This is an ugly hack, but it seems to work.
# When linking to a framework in Xcode, we need to be able to
# search for public headers within that framework
# "$SRCROOT/build-xcode/submodules/libFoo/$<CONFIG>/libFoo.framework/.."
# 
set(framework_search_paths
)

# Link to Cocoa Framework?
# set (link_to_cocoa 1)

# Enable ARC for Obj-C?
# set (enable_arc 1)


# =======================
# Leave the rest alone...
# =======================

# Configuration

set (My_Project_Version "${My_Project_Version_Major}.${My_Project_Version_Minor}.${My_Project_Version_Patch}")
set (My_Project_Copyright "Copyright © ${My_Project_Copyright_Date} ${My_Project_Author}.")

string(TOUPPER ${My_Project_Title} My_Project_Title_Caps  )
string(REGEX REPLACE " " "_" My_Project_Title_Caps ${My_Project_Title_Caps} )

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project (${My_Project_Title} VERSION "${My_Project_Version}")

# from http://stackoverflow.com/questions/25199677/how-to-detect-if-current-scope-has-a-parent-in-cmake
get_directory_property(hasParent PARENT_DIRECTORY)

if (hasParent)
else()
	# If building the framework independently, we canset a deployment target
	# set (CMAKE_OSX_DEPLOYMENT_TARGET "10.12")

	if (CMAKE_GENERATOR MATCHES "Xcode")
		set (CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
	endif()
endif()


# Search source directory
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/test)


# Search build directory (for version.h)
include_directories(${PROJECT_BINARY_DIR})


# Configure library/framework

add_library("${My_Project_Title}"
	STATIC
	${src_files}
	${private_headers}
	${public_headers}
	${resources}
)

string (REPLACE ";" " " framework_search_paths_string "${framework_search_paths}")

set_target_properties("${My_Project_Title}"
	PROPERTIES
	PREFIX ""
	FRAMEWORK_VERSION A
	MACOSX_FRAMEWORK_IDENTIFIER "${My_Project_Identifier}"
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${My_Project_Identifier}"
	XCODE_ATTRIBUTE_MARKETING_VERSION ${My_Project_Version}
	RESOURCE "${resources}"
	XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "${XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS} ${framework_search_paths_string}"
	PUBLIC_HEADER "${public_headers}"
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
)

if (hasParent)
else()
	set_target_properties("${My_Project_Title}"
		PROPERTIES
		XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] YES
		XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=MinSizeRel] NO
		XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=RelWithDebInfo] NO
		XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Release] NO
		XCODE_LINK_BUILD_PHASE_MODE "KNOWN_LOCATION"
	)
endif()

# Link to other libraries
target_link_libraries("${My_Project_Title}"
	${libraries_to_link}
)

# Link to Apple Cocoa Framework?
if (link_to_cocoa)
	target_link_libraries("${My_Project_Title}" "-framework Cocoa")
endif (link_to_cocoa)

# Enable ARC
if (enable_arc)
	set_property (TARGET "${My_Project_Title}"
		APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc"
	)
endif (enable_arc)



# ======================
# Process Template Files
# ======================

file(READ ${PROJECT_SOURCE_DIR}/LICENSE My_Project_License)

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


# ===================================
# Documentation Generation by doxygen
# ===================================

# Generate doxygen configuration file
string(REPLACE ";" " " doxygen_src_files "${src_files}" )
string(REPLACE ";" " " doxygen_header_files "${public_headers} {private_headers}" )

configure_file (
	"${PROJECT_SOURCE_DIR}/templates/doxygen.conf.in"
	"${PROJECT_BINARY_DIR}/doxygen.conf"
)


# ===========================================
# Build Test Suite with CuTest (unit testing)
# ===========================================

set(test_files
	test/CuTest.c
	test/CuTest.h
	${PROJECT_BINARY_DIR}/AllTests.c
)


# Process source files to look for tests to run
add_custom_command (
	OUTPUT ${PROJECT_BINARY_DIR}/AllTests.c
	COMMAND bash ${PROJECT_SOURCE_DIR}/test/make-tests.sh ${PROJECT_SOURCE_DIR}/src/*.c > ${PROJECT_BINARY_DIR}/AllTests.c
)


if(hasParent)
	# Don't create "run_tests" when we're a sub-project for something else
else()
	# Only create framework if we are the parent project
	set_target_properties("${My_Project_Title}"
		PROPERTIES
		FRAMEWORK TRUE
	)

	if (DEFINED TEST)
		add_definitions(-DTEST)

		add_executable(run_tests
			${src_files}
			${private_headers}
			${public_headers}
			${test_files}
		)

		# Link to other libraries
		target_link_libraries(run_tests
			${libraries_to_link}
		)

		# Link to Apple Cocoa Framework?
		if (link_to_cocoa)
			target_link_libraries(run_tests "-framework Cocoa")
		endif (link_to_cocoa)

		# Enable ARC
		if (enable_arc)
			set_property (TARGET run_tests
				APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc"
			)
		endif (enable_arc)

		enable_testing()

		add_test( test ${PROJECT_BINARY_DIR}/run_tests)

		# valgrind memory testing
	#	find_program (MEMORYCHECK_COMMAND valgrind)
	#	SET (MEMORYCHECK_COMMAND_OPTIONS --leak-check=full --error-exitcode=1)
	#
	#	add_test( memory_test ${MEMORYCHECK_COMMAND} ${MEMORYCHECK_COMMAND_OPTIONS} ${PROJECT_BINARY_DIR}/run_tests)

	endif()
endif()


# Build MultiMarkdown app
if (hasParent)
else()
	add_executable(multimarkdown
		src/d_string.c

		src/main.c
		src/argtable3.c

		${private_headers}
		${public_headers}
	)

	target_link_libraries(multimarkdown "${My_Project_Title}")
endif()


# ==========================
# Build Installer with CPack
# ==========================


if (WIN32)
	set (scripts
		scripts/mmd.bat
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

	texmf/tex/latex/mmd6/beamer/beamerthemekeynote-gradient.sty
	texmf/tex/latex/mmd6/beamer/beamerthemekeynote-portfolio.sty
	texmf/tex/latex/mmd6/beamer/beamerthemekeynote-vintage.sty
	texmf/tex/latex/mmd6/beamer/mmd6-beamer-begin.tex
	texmf/tex/latex/mmd6/beamer/mmd6-beamer-footer.tex
	texmf/tex/latex/mmd6/beamer/mmd6-beamer-leader.tex
	texmf/tex/latex/mmd6/beamer/Portfolio.png
	texmf/tex/latex/mmd6/beamer/Vintage.png

	texmf/tex/latex/mmd6/letterhead/mmd-envelope.sty
	texmf/tex/latex/mmd6/letterhead/mmd-letterhead.sty
	texmf/tex/latex/mmd6/letterhead/mmd6-letterhead-begin.tex
	texmf/tex/latex/mmd6/letterhead/mmd6-letterhead-footer.tex
	texmf/tex/latex/mmd6/letterhead/mmd6-letterhead-leader.tex

	texmf/tex/latex/mmd6/manuscript/manuscript.sty
	texmf/tex/latex/mmd6/manuscript/mmd6-manuscript-begin.tex
	texmf/tex/latex/mmd6/manuscript/mmd6-manuscript-footer.tex
	texmf/tex/latex/mmd6/manuscript/mmd6-manuscript-leader.tex

	texmf/tex/latex/mmd6/memoir-book/mmd6-memoir-book-begin.tex
	texmf/tex/latex/mmd6/memoir-book/mmd6-memoir-book-footer.tex
	texmf/tex/latex/mmd6/memoir-book/mmd6-memoir-book-leader.tex

	texmf/tex/latex/mmd6/shared/mmd6-criticmarkup.tex
	texmf/tex/latex/mmd6/shared/mmd6-default-metadata.tex
	texmf/tex/latex/mmd6/shared/mmd6-manuscript-packages.tex
	texmf/tex/latex/mmd6/shared/mmd6-memoir-copyright.tex
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


set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "${My_Project_Description}")
set (CPACK_PACKAGE_VERSION "${My_Project_Version_Major}.${My_Project_Version_Minor}.${My_Project_Version_Patch}")
set (CPACK_PACKAGE_VERSION_MAJOR "${My_Project_Version_Major}")
set (CPACK_PACKAGE_VERSION_MINOR "${My_Project_Version_Minor}")
set (CPACK_PACKAGE_VERSION_PATCH "My_Project_Version_Patch")

configure_file (
	"${PROJECT_SOURCE_DIR}/LICENSE"
	"${PROJECT_BINARY_DIR}/LICENSE.txt"
)
configure_file (
	"${PROJECT_SOURCE_DIR}/README.md"
	"${PROJECT_BINARY_DIR}/README.txt"
)

set (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/LICENSE.txt")
set (CPACK_RESOURCE_FILE_README "${PROJECT_BINARY_DIR}/README.txt")


if(hasParent)
else (hasParent)
	if (DEFINED TEST)
		# Don't install the test suite
	else()
		if (APPLE)
			# Apple Builds

			if (DEFINED ZIP)
				set (CPACK_GENERATOR ZIP)
			else (DEFINED ZIP)
				set (CPACK_GENERATOR productbuild)
			endif (DEFINED ZIP)
		else (APPLE)
			if (WIN32)
				# Windows Builds

				if (DEFINED ZIP)
					set (CPACK_GENERATOR ZIP)
				else (DEFINED ZIP)
					set (CPACK_GENERATOR NSIS)
				endif (DEFINED ZIP)
			elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
				# Linux
				set (CPACK_GENERATOR ZIP)
			endif (WIN32)
		endif (APPLE)

		# Install Target
		install (TARGETS multimarkdown
			DESTINATION bin
			COMPONENT MAIN
		)
		set (CPACK_COMPONENT_MAIN_DISPLAY_NAME "MultiMarkdown")
		set (CPACK_COMPONENT_MAIN_DESCRIPTION "Install the actual `multimarkdown` program.")

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
			install (TARGETS "${My_Project_Title}"
				ARCHIVE DESTINATION lib
				FRAMEWORK DESTINATION lib
				LIBRARY DESTINATION lib
				COMPONENT sharedlib
			)

			if (public_headers)
				# Install Headers
				install(FILES
					${public_headers}
					DESTINATION include/${My_Project_Title}
					COMPONENT sharedlib
				)
				set (CPACK_COMPONENT_SHAREDLIB_DISPLAY_NAME "Shared Library")
				set (CPACK_COMPONENT_SHAREDLIB_DESCRIPTION "MultiMarkdown shared library for use with third party software.")

				set (CPACK_COMPONENTS_ALL MAIN scripts sharedlib)
				set (CPACK_COMPONENT_SHAREDLIB_GROUP "libMultiMarkdown")
			else (public_headers)
				set (CPACK_COMPONENTS_ALL Main)
			endif (public_headers)
		endif (DEFINED SHAREDBUILD)

		# Install Documentation
		install(FILES
			"${PROJECT_BINARY_DIR}/LICENSE.txt"
			"${PROJECT_BINARY_DIR}/README.txt"
			COMPONENT Docs
			DESTINATION share/doc/MultiMarkdown
		)
		set (CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documentation")
		set (CPACK_COMPONENT_DOCS_DESCRIPTION "Install README and LICENSE.")

		# Install LaTeX support files
		install (FILES ${latex}
			DESTINATION share/texmf/tex/latex/mmd6
			COMPONENT latex
			PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
		)
		set (CPACK_COMPONENT_LATEX_DISPLAY_NAME "LaTeX support files")
		set (CPACK_COMPONENT_LATEX_DESCRIPTION "Install configuration files for MultiMarkdown-based LaTex configurations.")

		# Rename package
		if (APPLE)
			set (CPACK_PACKAGE_FILE_NAME "MultiMarkdown-Mac-${CPACK_PACKAGE_VERSION}")
		else (APPLE)
			set (CPACK_PACKAGE_FILE_NAME "MultiMarkdown-${CMAKE_SYSTEM_NAME}-${CPACK_PACKAGE_VERSION}")
		endif (APPLE)

		# Use default install locations (if appropriate)
		# TODO: This may need tweaking
		if (WIN32)
			set (CPACK_INSTALL_PREFIX "")
			set (CPACK_NSIS_MODIFY_PATH ON)
			if (ZIP)
					set(CPACK_SET_DESTDIR true)
			endif (ZIP)
		else (WIN32)
			set(CPACK_SET_DESTDIR true)
			set (CPACK_INSTALL_PREFIX /usr/local)
		endif (WIN32)

		set (CPACK_PACKAGE_CONTACT "fletcher@fletcherpenney.net")

	endif (DEFINED TEST)
endif (hasParent)

include(CPack)


# =====================================
# MultiMarkdown Integration Test Suite
# =====================================

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

ADD_MMD_TEST(mmd-6-beamer "-t beamer" Beamer tex)

ADD_MMD_TEST(mmd-6-memoir "-t memoir" Memoir tex)

ADD_MMD_TEST(mmd-6-fodt "-t fodt" MMD6Tests fodt)

ADD_MMD_TEST(mmd-6-opml "-t opml" MMD6Tests opml)

ADD_MMD_TEST(mmd-6-critic-accept "-a" CriticMarkup htmla)

ADD_MMD_TEST(mmd-6-critic-reject "-r" CriticMarkup htmlr)

# Some of these will (properly) fail.
# But it's useful to run manually at times to verify that
# round-tripping through OPML generally works

# add_test ( mmd-6-opml-parse 
# 		${PROJECT_SOURCE_DIR}/tests/MarkdownTest.pl
# 		--Script="${CMAKE_CURRENT_BINARY_DIR}/multimarkdown"
# 		--testdir=${PROJECT_SOURCE_DIR}/tests/MMD6Tests
# 		"--Flags=--opml -t mmd"
# 		--ext=text
# 		--source_ext=opml
# )


ADD_MMD_TEST(pathologic-compat "-c" ../build html)

ADD_MMD_TEST(pathologic "" ../build html)
