BUILD_DIR = build
DEBUG_DIR = build-test
XCODE_BUILD_DIR = build-xcode
XCODE_DEBUG_BUILD_DIR = build-xcode-test
DOC_DIR = documentation


# The release target will perform additional optimization
.PHONY : release
release: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DCMAKE_BUILD_TYPE=Release ..


# Also build a shared library
.PHONY : shared
shared: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DCMAKE_BUILD_TYPE=Release -DSHAREDBUILD=1 ..


# Create xcode project
# You can then build within XCode, or using the commands:
#	xcodebuild -configuration Debug
#	xcodebuild -configuration Release
.PHONY : xcode
xcode: $(XCODE_BUILD_DIR)
	cd $(XCODE_BUILD_DIR); \
	cmake -G Xcode ..


# Xcode debug variant
.PHONY : xcode-test
xcode-test: $(XCODE_DEBUG_BUILD_DIR)
	cd $(XCODE_DEBUG_BUILD_DIR); \
	cmake -G Xcode -DTEST=1 ..


# test target enables CuTest unit testing
.PHONY : test
test: $(DEBUG_DIR)
	cd $(DEBUG_DIR); \
	cmake -DTEST=1 -DCMAKE_BUILD_TYPE=Debug ..


# Build the documentation using doxygen
.PHONY : documentation
documentation:
	-mkdir $(DOC_DIR) 2>/dev/null; \
	cd $(DOC_DIR); \
	cmake -DDOCUMENTATION=1 ..; cd ..; \
	doxygen $(DOC_DIR)/doxygen.conf


# Build zip file package
.PHONY : zip
zip: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_BUILD_TYPE=Release -DZIP=1 ..


# Cross-compile for Windows using MinGW on *nix
.PHONY : windows
windows: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-MinGW-w64-64bit.cmake -DCMAKE_BUILD_TYPE=Release ..


# Cross-compile and build Windows zip file using MinGW on *nix
.PHONY : windows-zip
windows-zip: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-MinGW-w64-64bit.cmake -DCMAKE_BUILD_TYPE=Release -DZIP=1 ..


# Cross-compile for Windows using MinGW on *nix (32-bit)
.PHONY : windows-32
windows-32: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-MinGW-w64-32bit.cmake -DCMAKE_BUILD_TYPE=Release ..


# Cross-compile and build Windows zip file using MinGW on *nix (32-bit)
.PHONY : windows-zip-32
windows-zip-32: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-MinGW-w64-32bit.cmake -DCMAKE_BUILD_TYPE=Release -DZIP=1 ..


# Use astyle to format source code
.PHONY : astyle
astyle:
	astyle --options=.astylerc -q --recursive "src/*.h" "src/*.c"


# Generate a list of changes since last commit to 'master' branch
.PHONY : CHANGELOG
CHANGELOG:
	git log master..develop --format="*    %s" | sort | uniq > CHANGELOG-UNRELEASED


# Generate mapping of various enumeration constants (for debugging purposes)
.PHONY : map
map:
	cd $(BUILD_DIR); \
	../tools/enumsToPerl.pl ../src/libMultiMarkdown.h enumMap.txt;


# Create build directory if it doesn't exist
$(BUILD_DIR): CHANGELOG
	-mkdir $(BUILD_DIR) 2>/dev/null
	-cd $(BUILD_DIR); rm -rf *


# Create debug directory if it doesn't exist
$(DEBUG_DIR): CHANGELOG
	-mkdir $(DEBUG_DIR) 2>/dev/null
	-cd $(DEBUG_DIR); rm -rf *


# Build xcode directories if they don't exist
$(XCODE_BUILD_DIR): CHANGELOG
	-mkdir $(XCODE_BUILD_DIR) 2>/dev/null
	-cd $(XCODE_BUILD_DIR); rm -rf *

$(XCODE_DEBUG_BUILD_DIR): CHANGELOG
	-mkdir $(XCODE_DEBUG_BUILD_DIR) 2>/dev/null
	-cd $(XCODE_DEBUG_BUILD_DIR); rm -rf *


# Clean out the build directory
.PHONY : clean
clean:
	rm -rf $(BUILD_DIR)/*;

