#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <atomic>

#include "../dsp/spatial/BinauralLSDecoder.h"
#include "../dsp/spatial/EarlyReflections.h"
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

    // GUI keyboard state — the editor's MidiKeyboardComponent writes to this,
    // processBlock merges its notes into the host MIDI buffer each block.
    juce::MidiKeyboardState keyboardState;

    // Per-block peak readout for the editor's level meter. Audio thread
    // writes (relaxed, single-writer), message thread reads — atomic is the
    // only sync needed.
    std::atomic<float> outputPeakL { 0.0f };
    std::atomic<float> outputPeakR { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void pullParametersToVoices();
    void pullSpatialParameters();
    void handleMidi (const juce::MidiMessage& msg);

    void loadDefaultHRTFs (double sampleRate);

    VoiceManager voices;
    spatial::SimpleStereoDecoder  fallbackDecoder;
    spatial::SOFALoader           sofaLoader;
    spatial::BinauralLSDecoder    lsDecoder;
    spatial::EarlyReflections     earlyReflections;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauralJungleForgeProcessor)
};

} // namespace bjf
