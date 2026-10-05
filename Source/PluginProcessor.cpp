#include "PluginProcessor.h"
#include "PluginEditor.h"
VST3VocalChainDiagAudioProcessor::VST3VocalChainDiagAudioProcessor(): AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)) {}
bool VST3VocalChainDiagAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const { return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo() && l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo(); }
void VST3VocalChainDiagAudioProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer&) { juce::ScopedNoDenormals n; for(int c=getTotalNumInputChannels();c<getTotalNumOutputChannels();++c)b.clear(c,0,b.getNumSamples()); }
juce::AudioProcessorEditor* VST3VocalChainDiagAudioProcessor::createEditor(){ return new VST3VocalChainDiagEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new VST3VocalChainDiagAudioProcessor(); }
