#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace bjf
{
namespace param
{
    constexpr auto waveform = "waveform";
    constexpr auto attack   = "amp_attack";
    constexpr auto decay    = "amp_decay";
    constexpr auto sustain  = "amp_sustain";
    constexpr auto release  = "amp_release";
    constexpr auto gain     = "gain";
}

BinauralJungleForgeProcessor::BinauralJungleForgeProcessor()
    : AudioProcessor (BusesProperties()
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

BinauralJungleForgeProcessor::~BinauralJungleForgeProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
BinauralJungleForgeProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { param::waveform, 1 }, "Waveform",
        juce::StringArray { "Saw", "Square" }, 0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::attack, 1 }, "Attack",
        juce::NormalisableRange<float> { 0.001f, 4.0f, 0.0f, 0.3f }, 0.01f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::decay, 1 }, "Decay",
        juce::NormalisableRange<float> { 0.001f, 4.0f, 0.0f, 0.3f }, 0.2f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::sustain, 1 }, "Sustain",
        juce::NormalisableRange<float> { 0.0f, 1.0f }, 0.7f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::release, 1 }, "Release",
        juce::NormalisableRange<float> { 0.001f, 8.0f, 0.0f, 0.3f }, 0.5f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::gain, 1 }, "Gain",
        juce::NormalisableRange<float> { -60.0f, 6.0f, 0.01f }, -6.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    return layout;
}

void BinauralJungleForgeProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    voices.prepare (sampleRate);
    pullParametersToVoices();
}

void BinauralJungleForgeProcessor::releaseResources() {}

bool BinauralJungleForgeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    return mainOut == juce::AudioChannelSet::stereo()
        || mainOut == juce::AudioChannelSet::mono();
}

void BinauralJungleForgeProcessor::pullParametersToVoices()
{
    const auto wfIndex = static_cast<int> (
        apvts.getRawParameterValue (param::waveform)->load());

    voices.setWaveform (wfIndex == 0 ? Oscillator::Waveform::Saw
                                     : Oscillator::Waveform::Square);

    voices.setEnvelopeParameters (
        apvts.getRawParameterValue (param::attack)->load(),
        apvts.getRawParameterValue (param::decay)->load(),
        apvts.getRawParameterValue (param::sustain)->load(),
        apvts.getRawParameterValue (param::release)->load());
}

void BinauralJungleForgeProcessor::handleMidi (const juce::MidiMessage& msg)
{
    if (msg.isNoteOn())
        voices.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
    else if (msg.isNoteOff())
        voices.noteOff (msg.getNoteNumber());
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        voices.allNotesOff();
}

void BinauralJungleForgeProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                 juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numSamples  = buffer.getNumSamples();
    const auto numChannels = getTotalNumOutputChannels();

    for (int ch = 0; ch < numChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    pullParametersToVoices();

    const auto gainLin = juce::Decibels::decibelsToGain (
        apvts.getRawParameterValue (param::gain)->load());

    auto midiIt = midiMessages.cbegin();
    int sampleIndex = 0;

    while (sampleIndex < numSamples)
    {
        int nextEventOffset = numSamples;
        if (midiIt != midiMessages.cend())
            nextEventOffset = juce::jlimit (sampleIndex, numSamples,
                                            (*midiIt).samplePosition);

        for (int i = sampleIndex; i < nextEventOffset; ++i)
        {
            const auto sample = voices.renderNextSample() * gainLin;
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.setSample (ch, i, sample);
        }

        sampleIndex = nextEventOffset;

        while (midiIt != midiMessages.cend()
               && (*midiIt).samplePosition <= sampleIndex)
        {
            handleMidi ((*midiIt).getMessage());
            ++midiIt;
        }
    }
}

juce::AudioProcessorEditor* BinauralJungleForgeProcessor::createEditor()
{
    return new BinauralJungleForgeEditor (*this);
}

void BinauralJungleForgeProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void BinauralJungleForgeProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

} // namespace bjf

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new bjf::BinauralJungleForgeProcessor();
}
