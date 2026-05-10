PROJECT_PATH := "/Users/kalimeeks/Documents/Claude/Projects/MASTERING SWEET/MasteringSuite"
TEST_DIR := "$(PROJECT_PATH)/tests"
BUILD_DIR := "$(TEST_DIR)/build"

MACOS_SDK := $(shell xcrun --sdk macosx --show-sdk-path)
XCODE_PATH := /Applications/Xcode.app/Contents/Developer

CXX := "$(XCODE_PATH)/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
CXXFLAGS := -isysroot "$(MACOS_SDK)" \
	-I"$(PROJECT_PATH)/Source" \
	-fPIC -std=c++17 -O2 -DNDEBUG

LDFLAGS := -lSystem

SRC_FILES := "$(PROJECT_PATH)/Source/Metering/EbuR128Meter_TestImpl.cpp" \
	"$(TEST_DIR)/MeterValidation_NoJUCE.cpp"

TARGET := "$(BUILD_DIR)/MeterValidation_Test"

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	@mkdir -p "$(BUILD_DIR)"

$(TARGET): $(SRC_FILES)
	@echo "Building meter validation test (no JUCE dependencies)..."
	$(CXX) $(CXXFLAGS) $(SRC_FILES) -o $@ $(LDFLAGS)
	@echo "✅ Test compiled: $@"
	@echo ""
	@echo "Running validation..."
	@$@

clean:
	rm -rf "$(BUILD_DIR)"

.PHONY: all clean
