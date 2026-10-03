cmake_minimum_required(VERSION 3.22)
project(808YUROExact VERSION 1.1.0 LANGUAGES CXX)

# WebView2 is used so the original HTML/CSS layout is rendered inside the VST3.
execute_process(
    COMMAND nuget install Microsoft.Web.WebView2 -OutputDirectory "${CMAKE_BINARY_DIR}/nuget"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE NUGET_RESULT
)
if (NOT NUGET_RESULT EQUAL 0)
    message(FATAL_ERROR "Could not install Microsoft.Web.WebView2 with NuGet")
endif()

set(JUCE_WEBVIEW2_PACKAGE_LOCATION "${CMAKE_BINARY_DIR}/nuget")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(YuroWebUI
    SOURCES Resources/index.html
)

juce_add_plugin(808YUROExact
    COMPANY_NAME "29YURO"
    PLUGIN_MANUFACTURER_CODE YURO
    PLUGIN_CODE UEXA
    FORMATS VST3
    PRODUCT_NAME "VST3Effekts+"
    VERSION 1.0.0
    DESCRIPTION "VST3Effekts+ — 29YURO vocal transition effects"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    NEEDS_WEBVIEW2 TRUE
)

target_sources(808YUROExact PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_features(808YUROExact PRIVATE cxx_std_17)

target_compile_definitions(808YUROExact PRIVATE
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(808YUROExact PRIVATE
    YuroWebUI
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
