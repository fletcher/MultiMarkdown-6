BUILD_DIR = build

$(BUILD_DIR_):
	-mkdir $(BUILD_DIR_) 2>/dev/null
	-cd $(BUILD_DIR); rm -rf *

# The release target will perform additional optimization
release: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DCMAKE_BUILD_TYPE=Release ..

# Enables CuTest unit testing
debug: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DTEST=1 ..

# For Mac only
xcode: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -G Xcode ..

# Cross-compile for Windows
windows: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-mingw32.cmake -DCMAKE_BUILD_TYPE=Release ..

# Clean out the build directory
clean:
	rm -rf $(BUILD_DIR)/*
