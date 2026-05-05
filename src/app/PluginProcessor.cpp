#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace bjf
{
namespace param
{
    constexpr auto waveform     = "waveform";
    constexpr auto stackSize    = "osc_stack_size";
    constexpr auto detuneCents  = "osc_detune_cents";
    constexpr auto filterType   = "filter_type";
    constexpr auto filterCutoff = "filter_cutoff";
    constexpr auto filterReso   = "filter_resonance";
    constexpr auto attack       = "amp_attack";
    constexpr auto decay        = "amp_decay";
    constexpr auto sustain      = "amp_sustain";
    constexpr auto release      = "amp_release";
    constexpr auto spatialAz    = "spatial_az";
    constexpr auto spatialEl    = "spatial_el";
    constexpr auto gain         = "gain";
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

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { param::stackSize, 1 }, "Stack Size",
        1, OscillatorStack::kMaxOscs, 3));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::detuneCents, 1 }, "Detune",
        juce::NormalisableRange<float> { 0.0f, 50.0f }, 7.0f,
        juce::AudioParameterFloatAttributes().withLabel ("cents")));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { param::filterType, 1 }, "Filter Type",
        juce::StringArray { "Low Pass", "Band Pass", "High Pass" }, 0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::filterCutoff, 1 }, "Cutoff",
        juce::NormalisableRange<float> { 20.0f, 20000.0f, 0.0f, 0.25f }, 1200.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::filterReso, 1 }, "Resonance",
        juce::NormalisableRange<float> { 0.1f, 10.0f, 0.0f, 0.5f }, 0.707f));

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
        juce::ParameterID { param::spatialAz, 1 }, "Azimuth",
        juce::NormalisableRange<float> { -180.0f, 180.0f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("\xc2\xb0"))); // °

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::spatialEl, 1 }, "Elevation",
        juce::NormalisableRange<float> { -90.0f, 90.0f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("\xc2\xb0")));

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
    pullSpatialParameters();
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

    voices.setStackSize (static_cast<int> (
        apvts.getRawParameterValue (param::stackSize)->load()));
    voices.setDetuneCents (apvts.getRawParameterValue (param::detuneCents)->load());

    const auto ftIndex = static_cast<int> (
        apvts.getRawParameterValue (param::filterType)->load());
    voices.setFilterType (ftIndex == 0 ? Filter::Type::LowPass
                          : ftIndex == 1 ? Filter::Type::BandPass
                                         : Filter::Type::HighPass);

    voices.setFilterCutoff    (apvts.getRawParameterValue (param::filterCutoff)->load());
    voices.setFilterResonance (apvts.getRawParameterValue (param::filterReso)->load());

    voices.setEnvelopeParameters (
        apvts.getRawParameterValue (param::attack)->load(),
        apvts.getRawParameterValue (param::decay)->load(),
        apvts.getRawParameterValue (param::sustain)->load(),
        apvts.getRawParameterValue (param::release)->load());
}

void BinauralJungleForgeProcessor::pullSpatialParameters()
{
    constexpr float deg2rad = 0.017453292519943295f;
    const auto azDeg = apvts.getRawParameterValue (param::spatialAz)->load();
    const auto elDeg = apvts.getRawParameterValue (param::spatialEl)->load();
    encoder.setPosition (azDeg * deg2rad, elDeg * deg2rad);
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
    pullSpatialParameters();

    const auto gainLin = juce::Decibels::decibelsToGain (
        apvts.getRawParameterValue (param::gain)->load());

    // Spatial scratch — stack-allocated, per-sample scratchpad. Allocates
    // nothing on the audio thread.
    float hoaSample[spatial::kNumHoaChannels];

    auto* outL = buffer.getWritePointer (0);
    auto* outR = (numChannels > 1) ? buffer.getWritePointer (1) : nullptr;

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
            const auto mono = voices.renderNextSample() * gainLin;

            encoder.encodeSample (mono, hoaSample);

            float l = 0.0f, r = 0.0f;
            decoder.decodeSample (hoaSample, l, r);

            if (outR != nullptr)
            {
                outL[i] = l;
                outR[i] = r;
            }
            else
            {
                outL[i] = 0.5f * (l + r);
            }
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
