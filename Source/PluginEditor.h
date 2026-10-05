#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class VST3VocalChainDiagEditor final : public juce::AudioProcessorEditor {
public: explicit VST3VocalChainDiagEditor(VST3VocalChainDiagAudioProcessor& p):AudioProcessorEditor(&p){setSize(620,260);} void paint(juce::Graphics&) override; void resized() override {}
};
