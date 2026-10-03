#pragma once
#include <JuceHeader.h>
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
