cmake_minimum_required(VERSION 3.22)
project(MasteringSuite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_PATH "$ENV{HOME}/JUCE")

# Add JUCE
add_subdirectory("${JUCE_PATH}/extras/cmake" JUCE)

# Create meter test executable
juce_add_console_app(MeterValidation_Test)

target_sources(MeterValidation_Test PRIVATE
    "Source/Metering/EbuR128Meter.h"
    "Source/Metering/EbuR128Meter.cpp"
    "tests/MeterValidation.cpp"
)

target_include_directories(MeterValidation_Test PRIVATE
    "Source"
)

target_link_libraries(MeterValidation_Test PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_dsp
)
