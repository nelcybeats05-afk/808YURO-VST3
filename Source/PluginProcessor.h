#pragma once
#include <JuceHeader.h>

class VocalChainPlusAudioProcessor final : public juce::AudioProcessor
{
public:
    VocalChainPlusAudioProcessor();
    ~VocalChainPlusAudioProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> inputDb{-60.0f}, outputDb{-60.0f}, pitchHz{0.0f}, timingScore{0.0f};
    juce::String detectedNote() const;
    void loadPreset(int);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Reverb reverb;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay{192000};
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    StereoFilter lowEq, midEq, highEq;
    double sr = 44100.0;
    float env = 0.0f, deEssEnv = 0.0f;
    int samplesSinceTransient = 0;
    std::vector<float> analysisRing;
    size_t analysisWrite = 0;

    // Two-head time-domain pitch shifter. Intended for vocal correction/effect, not formant preservation.
    std::vector<std::vector<float>> pitchBuffer;
    int pitchWrite = 0;
    float pitchPhase = 0.0f;

    void updateAnalysis(const juce::AudioBuffer<float>&);
    void applyDeEsser(juce::AudioBuffer<float>&, float amount);
    void applyWidth(juce::AudioBuffer<float>&, float amount);
    void applyPitchShift(juce::AudioBuffer<float>&, float semitones, float blend);
    float targetCorrectionSemitones(float detectedHz) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalChainPlusAudioProcessor)
};
