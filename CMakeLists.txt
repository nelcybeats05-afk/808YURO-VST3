cmake_minimum_required(VERSION 3.22)
project(VST3VocalChainDiag VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.8)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(VST3VocalChainDiag
    COMPANY_NAME "29YURO"
    BUNDLE_ID "com.29yuro.vst3vocalchain.diag"
    PLUGIN_MANUFACTURER_CODE Yuro
    PLUGIN_CODE VcD1
    FORMATS VST3
    PRODUCT_NAME "VST3VocalChain+"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)
juce_generate_juce_header(VST3VocalChainDiag)
target_sources(VST3VocalChainDiag PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h)
target_compile_definitions(VST3VocalChainDiag PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(VST3VocalChainDiag PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
