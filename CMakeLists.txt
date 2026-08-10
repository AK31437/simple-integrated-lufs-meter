cmake_minimum_required(VERSION 3.22)
project(LUFSMeter VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(LUFSMeter
    COMPANY_NAME "Anonymous"
    PLUGIN_MANUFACTURER_CODE Lufs
    PLUGIN_CODE Lufs
    FORMATS VST3
    PRODUCT_NAME "Simple Integrated LUFS Meter"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES "Fx" "Analyser"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(LUFSMeter PRIVATE
    Source/LufsCalculator.cpp
    Source/LufsMeterProcessor.cpp
    Source/LufsMeterEditor.cpp
)

target_compile_definitions(LUFSMeter PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(LUFSMeter PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
