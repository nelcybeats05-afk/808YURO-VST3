#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class YuroExactProcessor final : public juce::AudioProcessor
{
public:
    YuroExactProcessor();
    ~YuroExactProcessor() override = default;

    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "808YURO Riser"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    // Automatic mixer-input recording: arm, play FL, silence/stop finishes the take.
    void armCapture() noexcept;
    void stopCapture() noexcept;
    bool captureReady() const noexcept { return captureState.load() == 3; }
    bool isCapturing() const noexcept { return captureState.load() == 2; }
    bool isArmed() const noexcept { return captureState.load() == 1; }
    juce::File writeCapturedWav(); // call on UI thread only after captureReady

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();

private:
    std::atomic<int> captureState { 0 }; // 0 idle, 1 armed, 2 recording, 3 finished
    juce::AudioBuffer<float> capturedAudio;
    int capturedFrames = 0; // only audio thread writes until state 3
    int quietFrames = 0;
    double sr = 44100.0;
    juce::dsp::DelayLine<float> delay { 192000 };
    juce::dsp::IIR::Filter<float> lpL, lpR, hpL, hpR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YuroExactProcessor)
};
