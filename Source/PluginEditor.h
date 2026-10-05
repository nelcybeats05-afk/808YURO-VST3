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
    juce::ComboBox style,key,scale;
    juce::TextEditor artist;
    juce::TextButton applyArtist{"APPLY ARTIST CHAIN"}, analyse{"ANALYSE"}, ai{"AI ON"}, reference{"LOAD TRACK"}, refAnalyse{"ANALYSE TRACK"}, refRemove{"REMOVE"};
    juce::Label status,pitch,levels,title,subtitle,refInfo,artistInfo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> styleA,keyA,scaleA;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::File referenceFile;
    void addKnob(const juce::String&,const juce::String&);
    void timerCallback() override;
    void applyArtistChain();
    void analyseReference();
    void styleButton(juce::TextButton&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalChainPlusAudioProcessorEditor)
};
