#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
class YuroExactProcessorEditor final : public juce::AudioProcessorEditor {
public:
 explicit YuroExactProcessorEditor(YuroExactProcessor&);
 ~YuroExactProcessorEditor() override = default;
 void resized() override;
private:
 YuroExactProcessor& processor;
 juce::WebBrowserComponent browser;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YuroExactProcessorEditor)
};
