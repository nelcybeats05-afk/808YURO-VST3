#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VocalChainPlusAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VocalChainPlusAudioProcessorEditor(VocalChainPlusAudioProcessor&);
    ~VocalChainPlusAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    VocalChainPlusAudioProcessor& p;
    struct Knob { juce::Slider s; juce::Label l; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; };
    std::vector<std::unique_ptr<Knob>> knobs;
    juce::ComboBox style, role, key, scale; juce::TextButton analyse{"ANALYSE"}, ai{"AI ON"}, reference{"LOAD REFERENCE"};
    juce::Label status, pitch, levels, title, subtitle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> styleA,roleA,keyA,scaleA;
    std::unique_ptr<juce::FileChooser> chooser; juce::String refName{"No reference loaded"};
    void addKnob(const juce::String&,const juce::String&); void timerCallback() override; void updateStyle();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalChainPlusAudioProcessorEditor)
};
