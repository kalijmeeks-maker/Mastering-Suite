#!/bin/bash
set -e

JUCE_PATH="$HOME/JUCE"
PROJECT_PATH="/Users/kalimeeks/Documents/Claude/Projects/MASTERING SWEET/MasteringSuite"
JUCE_LIB_CODE="$PROJECT_PATH/JuceLibraryCode"
TEST_DIR="$PROJECT_PATH/tests"
BUILD_DIR="$TEST_DIR/build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Building meter validation test..."
echo ""

# Compile test (includes JuceHeader via JuceLibraryCode)
MACOS_SDK=$(xcrun --sdk macosx --show-sdk-path)
clang++ \
  -isysroot "$MACOS_SDK" \
  -I"$JUCE_PATH/modules" \
  -I"$JUCE_LIB_CODE" \
  -I"$PROJECT_PATH/Source" \
  -fPIC -std=c++17 -O2 -DNDEBUG \
  "$PROJECT_PATH/Source/Metering/EbuR128Meter.cpp" \
  "$TEST_DIR/MeterValidation.cpp" \
  -o MeterValidation_Test \
  -lSystem

echo "✅ Test compiled: $BUILD_DIR/MeterValidation_Test"
echo ""
echo "Running validation..."
"$BUILD_DIR/MeterValidation_Test"
