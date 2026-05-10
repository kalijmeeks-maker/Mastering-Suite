#!/bin/bash
set -e

PROJECT_PATH="/Users/kalimeeks/Documents/Claude/Projects/MASTERING SWEET/MasteringSuite"
TEST_DIR="$PROJECT_PATH/tests"
BUILD_DIR="$TEST_DIR/build"

mkdir -p "$BUILD_DIR"

MACOS_SDK=$(xcrun --sdk macosx --show-sdk-path)
XCODE_PATH="/Applications/Xcode.app/Contents/Developer"
CXX="$XCODE_PATH/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"

SRC1="$PROJECT_PATH/Source/Metering/EbuR128Meter_TestImpl.cpp"
SRC2="$TEST_DIR/MeterValidation_NoJUCE.cpp"
TARGET="$BUILD_DIR/MeterValidation_Test"

echo "Building meter validation test (no JUCE dependencies)..."
$CXX -isysroot "$MACOS_SDK" \
  -I"$PROJECT_PATH/Source" \
  -fPIC -std=c++17 -O2 -DNDEBUG \
  "$SRC1" "$SRC2" \
  -o "$TARGET" \
  -lSystem

echo "✅ Test compiled: $TARGET"
echo ""
echo "Running validation..."
"$TARGET"
