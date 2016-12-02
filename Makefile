BUILD_DIR = build
XCODE_BUILD_DIR = build-xcode
XCODE_DEBUG_BUILD_DIR = build-xcode-debug

# The release target will perform additional optimization
.PHONY : release
release: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DCMAKE_BUILD_TYPE=Release ..

# Build zip file package
.PHONY : zip
zip: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_BUILD_TYPE=Release -DZIP=1 ..

# debug target enables CuTest unit testing
.PHONY : debug
debug: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DTEST=1 ..

# analyze target enables use of clang's scan-build (if installed)
# will then need to run 'scan-build make' to compile and analyze
# 'scan-build -V make' will show the results graphically in your
# web browser
.PHONY : analyze
analyze: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	scan-build cmake -DTEST=1 ..

# Create xcode project
# You can then build within XCode, or using the commands:
#	xcodebuild -configuration Debug
#	xcodebuild -configuration Release
.PHONY : xcode
xcode: $(XCOD_BUILD_DIR)
	cd $(XCOD_BUILD_DIR); \
	cmake -G Xcode ..

.PHONY : xcode-debug
xcode-debug: $(XCODE_DEBUG_BUILD_DIR)
	cd $(XCODE_DEBUG_BUILD_DIR); \
	cmake -G Xcode -DTEST=1 ..

# Cross-compile for Windows using MinGW on *nix
.PHONY : windows
windows: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-MinGW-w64-64bit.cmake -DCMAKE_BUILD_TYPE=Release ..

# Build Windows zip file using MinGW on *nix
.PHONY : windows-zip
windows-zip: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-MinGW-w64-64bit.cmake -DCMAKE_BUILD_TYPE=Release -DZIP=1 ..

# Cross-compile for Windows using MinGW on *nix (32-bit)
.PHONY : windows-32
windows-32: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-MinGW-w64-32bit.cmake -DCMAKE_BUILD_TYPE=Release ..

# Build Windows zip file using MinGW on *nix (32-bit)
.PHONY : windows-zip-32
windows-zip-32: $(BUILD_DIR)
	cd $(BUILD_DIR); touch README.html; \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-mingw32.cmake -DCMAKE_BUILD_TYPE=Release -DZIP=1 ..

# Build the documentation using doxygen
.PHONY : documentation
documentation: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DDOCUMENTATION=1 ..; cd ..; \
	doxygen build/doxygen.conf

.PHONY : gh-pages
gh-pages: documentation
	cp -r $(BUILD_DIR)/documentation/html/* documentation/

# Clean out the build directory
.PHONY : clean
clean:
	rm -rf $(BUILD_DIR)/*

# Create build directory if it doesn't exist
$(BUILD_DIR): CHANGELOG
	-mkdir $(BUILD_DIR) 2>/dev/null
	-cd $(BUILD_DIR); rm -rf *

# Build xcode directories if they don't exist
$(XCODE_BUILD_DIR):
	-mkdir $(XCODE_BUILD_DIR) 2>/dev/null
	-cd $(XCODE_BUILD_DIR); rm -rf *

$(XCODE_DEBUG_BUILD_DIR):
	-mkdir $(XCODE_DEBUG_BUILD_DIR) 2>/dev/null
	-cd $(XCODE_DEBUG_BUILD_DIR); rm -rf *

# Generate a list of changes since last commit to 'master' branch
.PHONY : CHANGELOG
CHANGELOG:
	git log master..develop --format="*    %s" | sort | uniq > CHANGELOG-UNRELEASED

