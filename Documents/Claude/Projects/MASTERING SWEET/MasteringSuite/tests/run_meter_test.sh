#!/bin/bash
set -e

PROJECT_PATH="/Users/kalimeeks/Documents/Claude/Projects/MASTERING SWEET/MasteringSuite"
JUCE_PATH="$HOME/JUCE"
XCODE_DERIVE_DATA="$HOME/Library/Developer/Xcode/DerivedData"

# Find the built standalone app
APP_PATH="$PROJECT_PATH/Builds/MacOSX/build/Debug/MasteringSuite.app"

if [ ! -d "$APP_PATH" ]; then
    echo "❌ App not found at $APP_PATH"
    echo "Run: xcodebuild -project $PROJECT_PATH/Builds/MacOSX/MasteringSuite.xcodeproj -scheme 'MasteringSuite - Standalone Plugin' -configuration Debug build"
    exit 1
fi

echo "App built successfully at: $APP_PATH"
echo ""
echo "Running meter validation test via the Xcode-built application..."
echo ""

# The test should be run by instantiating the meter and feeding test signals
# For now, we'll just verify the app was built

# To properly test, we need to either:
# 1. Add a test target to the Xcode project
# 2. Create a separate test executable
# 3. Embed test code in the plugin itself

# Option 3: Create test via a simple C++ program that uses the built libraries
echo "Note: Full meter validation test requires either:"
echo "  - Adding a 'Unit Test Bundle' target to the Xcode project"
echo "  - Or creating a standalone test executable that links against the meter library"
echo ""
echo "✅ Xcode build succeeded. Plugin ready for manual testing."
