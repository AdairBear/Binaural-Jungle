#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "../dsp/spatial/BinauralLSDecoder.h"
#include "../dsp/spatial/HOAEncoder.h"
#include "../dsp/spatial/SimpleStereoDecoder.h"
#include "../dsp/spatial/SOFALoader.h"
#include "../dsp/voice/VoiceManager.h"

namespace bjf
{

class BinauralJungleForgeProcessor final : public juce::AudioProcessor
{
public:
    BinauralJungleForgeProcessor();
    ~BinauralJungleForgeProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>& buffer,
                       juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void pullParametersToVoices();
    void pullSpatialParameters();
    void handleMidi (const juce::MidiMessage& msg);

    void loadDefaultHRTFs (double sampleRate);

    VoiceManager voices;
    spatial::HOAEncoder           encoder;
    spatial::SimpleStereoDecoder  fallbackDecoder;
    spatial::SOFALoader           sofaLoader;
    spatial::BinauralLSDecoder    lsDecoder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauralJungleForgeProcessor)
};

} // namespace bjf
