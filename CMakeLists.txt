cmake_minimum_required(VERSION 3.22)
project(VocalChainPlus VERSION 2.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.8)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(VocalChainPlus
    COMPANY_NAME "29YURO"
    BUNDLE_ID "com.29yuro.vocalchainplus"
    PLUGIN_MANUFACTURER_CODE Yuro
    PLUGIN_CODE VcP2
    FORMATS VST3
    PRODUCT_NAME "VST3 VocalChain+ by 29YURO"
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(VocalChainPlus)
target_sources(VocalChainPlus PRIVATE
    Source/PluginProcessor.cpp Source/PluginProcessor.h
    Source/PluginEditor.cpp Source/PluginEditor.h)
target_compile_definitions(VocalChainPlus PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(VocalChainPlus PRIVATE
    juce::juce_audio_utils juce::juce_audio_formats juce::juce_dsp juce::juce_gui_extra
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
