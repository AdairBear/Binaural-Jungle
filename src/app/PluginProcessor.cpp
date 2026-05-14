#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

#include "../dsp/spatial/DecoderMatrix.h"

#ifndef BJF_DEFAULT_SOFA_PATH
 #define BJF_DEFAULT_SOFA_PATH ""
#endif

namespace bjf
{

BinauralJungleForgeProcessor::BinauralJungleForgeProcessor()
    : AudioProcessor (BusesProperties()
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout()),
      presetManager (apvts)
{
}

BinauralJungleForgeProcessor::~BinauralJungleForgeProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
BinauralJungleForgeProcessor::createParameterLayout()
{
    using FloatRange    = juce::NormalisableRange<float>;
    using FloatAttrs    = juce::AudioParameterFloatAttributes;
    using ParameterID   = juce::ParameterID;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addChoice = [&] (const char* id, const juce::String& name,
                          juce::StringArray choices, int defaultIndex)
    {
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            ParameterID { id, 1 }, name, std::move (choices), defaultIndex));
    };
    auto addInt = [&] (const char* id, const juce::String& name,
                       int lo, int hi, int dflt)
    {
        layout.add (std::make_unique<juce::AudioParameterInt> (
            ParameterID { id, 1 }, name, lo, hi, dflt));
    };
    auto addFloat = [&] (const char* id, const juce::String& name,
                         FloatRange range, float dflt,
                         const juce::String& unit = {})
    {
        FloatAttrs attrs;
        if (unit.isNotEmpty()) attrs = attrs.withLabel (unit);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            ParameterID { id, 1 }, name, range, dflt, attrs));
    };

    // ── Oscillator ─────────────────────────────────────────────────────────
    addChoice (pid::waveform,     "Waveform",  { "Saw", "Square" }, 0);
    addInt    (pid::oscOctave,    "Octave",    -2, 2, 0);
    addFloat  (pid::detuneCents,  "Detune",    FloatRange { 0.0f, 50.0f }, 7.0f, "cents");
    addFloat  (pid::oscPhase,     "Phase",     FloatRange { 0.0f, 1.0f }, 0.0f);
    addInt    (pid::stackSize,    "Stack Size", 1, OscillatorStack::kMaxOscs, 3);
    addFloat  (pid::oscSpread,    "Spread",    FloatRange { 0.0f, 1.0f }, 0.3f);

    // ── Granular ───────────────────────────────────────────────────────────
    addChoice (pid::granSample,   "Sample",
               { "—", "Slot 1", "Slot 2", "Slot 3", "Slot 4" }, 0);
    addFloat  (pid::granDensity,  "Density",  FloatRange { 0.0f, 1.0f }, 0.4f);
    addFloat  (pid::granSizeMs,   "Grain",    FloatRange { 1.0f, 500.0f, 0.0f, 0.4f }, 60.0f, "ms");
    addFloat  (pid::granPitch,    "Pitch",    FloatRange { -24.0f, 24.0f, 0.01f }, 0.0f, "st");
    addFloat  (pid::granScatter,  "Scatter",  FloatRange { 0.0f, 1.0f }, 0.2f);
    addFloat  (pid::granSpray,    "Spray",    FloatRange { 0.0f, 1.0f }, 0.1f);
    addFloat  (pid::granMix,      "Mix",      FloatRange { 0.0f, 1.0f }, 0.0f);

    // ── Filter ─────────────────────────────────────────────────────────────
    addChoice (pid::filterType,      "Filter Type",
               { "Low Pass", "Band Pass", "High Pass" }, 0);
    addFloat  (pid::filterCutoff,    "Cutoff",
               FloatRange { 20.0f, 20000.0f, 0.0f, 0.25f }, 1200.0f, "Hz");
    addFloat  (pid::filterReso,      "Resonance",
               FloatRange { 0.1f, 10.0f, 0.0f, 0.5f }, 0.707f);
    addFloat  (pid::filterEnvAmount, "Env Amount", FloatRange { -1.0f, 1.0f }, 0.0f);
    addFloat  (pid::filterKeyTrack,  "Key Track",  FloatRange {  0.0f, 1.0f }, 0.0f);
    addFloat  (pid::filterDrive,     "Drive",      FloatRange {  0.0f, 1.0f }, 0.0f);

    // ── Amp envelope (DAHDSR) ──────────────────────────────────────────────
    addFloat  (pid::envDelay, "Delay",   FloatRange { 0.0f,  2.0f, 0.0f, 0.3f }, 0.0f,  "s");
    addFloat  (pid::attack,   "Attack",  FloatRange { 0.001f, 4.0f, 0.0f, 0.3f }, 0.01f, "s");
    addFloat  (pid::envHold,  "Hold",    FloatRange { 0.0f,  2.0f, 0.0f, 0.3f }, 0.0f,  "s");
    addFloat  (pid::decay,    "Decay",   FloatRange { 0.001f, 4.0f, 0.0f, 0.3f }, 0.2f,  "s");
    addFloat  (pid::sustain,  "Sustain", FloatRange { 0.0f,  1.0f }, 0.7f);
    addFloat  (pid::release,  "Release", FloatRange { 0.001f, 8.0f, 0.0f, 0.3f }, 0.5f,  "s");
    addFloat  (pid::envCurve, "Curve",   FloatRange { 0.0f,  1.0f }, 0.5f);

    // ── Spatial ────────────────────────────────────────────────────────────
    addFloat  (pid::spatialAz,       "Azimuth",
               FloatRange { -180.0f, 180.0f }, 0.0f, "\xc2\xb0");
    addFloat  (pid::spatialEl,       "Elevation",
               FloatRange { -90.0f, 90.0f }, 0.0f, "\xc2\xb0");
    addFloat  (pid::spatialSpreadAz, "Spread Azimuth",
               FloatRange { 0.0f, 360.0f }, 0.0f, "\xc2\xb0");
    addFloat  (pid::spatialSpreadEl, "Spread Elevation",
               FloatRange { 0.0f, 180.0f }, 0.0f, "\xc2\xb0");

    // ── Early reflections ──────────────────────────────────────────────────
    addFloat  (pid::erRoomSize,    "ER Room Size",
               FloatRange { 1.0f, 30.0f, 0.0f, 0.5f }, 8.0f, "m");
    addFloat  (pid::erWallDamping, "ER Wall Damping", FloatRange { 0.0f, 1.0f }, 0.3f);
    addFloat  (pid::erMix,         "ER Mix",          FloatRange { 0.0f, 1.0f }, 0.0f);

    // ── HOA diffuse reverb (panel-only until DSP is wired) ─────────────────
    addFloat  (pid::revRoomSize,  "Reverb Room",      FloatRange { 0.0f, 1.0f }, 0.5f);
    addFloat  (pid::revDecay,     "Reverb Decay",     FloatRange { 0.2f, 12.0f, 0.0f, 0.4f }, 2.0f, "s");
    addFloat  (pid::revDamping,   "Reverb Damping",   FloatRange { 0.0f, 1.0f }, 0.4f);
    addFloat  (pid::revPreDelay,  "Reverb Pre-Delay", FloatRange { 0.0f, 250.0f, 0.0f, 0.4f }, 20.0f, "ms");
    addFloat  (pid::revDiffusion, "Reverb Diffusion", FloatRange { 0.0f, 1.0f }, 0.7f);
    addFloat  (pid::revWetDry,    "Reverb Wet/Dry",   FloatRange { 0.0f, 1.0f }, 0.25f);

    // ── Master ─────────────────────────────────────────────────────────────
    addFloat  (pid::gain,   "Gain", FloatRange { -60.0f, 6.0f, 0.01f }, -6.0f, "dB");
    addFloat  (pid::pan,    "Pan",  FloatRange { -1.0f, 1.0f }, 0.0f);
    addInt    (pid::voices, "Voices", 1, VoiceManager::kMaxVoices, VoiceManager::kMaxVoices);
    addFloat  (pid::bpm,    "BPM",  FloatRange { 40.0f, 240.0f }, 120.0f, "bpm");

    return layout;
}

void BinauralJungleForgeProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voices.prepare (sampleRate);
    pullParametersToVoices();
    pullSpatialParameters();

    earlyReflections.prepare (sampleRate);

    lsDecoder.prepare (sampleRate, samplesPerBlock);
    loadDefaultHRTFs (sampleRate);
}

void BinauralJungleForgeProcessor::loadDefaultHRTFs (double /*sampleRate*/)
{
    const juce::String defaultPath { BJF_DEFAULT_SOFA_PATH };
    if (defaultPath.isEmpty())
        return;

    const auto status = sofaLoader.loadFromFile (defaultPath.toStdString());
    if (! status.success)
    {
        juce::Logger::writeToLog ("BJF: SOFA load failed (" + juce::String (status.message) + ")");
        return;
    }

    const int M = sofaLoader.getNumDirections();
    const int N = sofaLoader.getFilterLength();
    const int R = sofaLoader.getNumReceivers();

    std::vector<float> dirs (static_cast<std::size_t> (M) * 2);
    for (int m = 0; m < M; ++m)
    {
        float az = 0.0f, el = 0.0f;
        sofaLoader.getDirection (m, az, el);
        dirs[static_cast<std::size_t> (m) * 2 + 0] = az;
        dirs[static_cast<std::size_t> (m) * 2 + 1] = el;
    }

    std::vector<float> hrirs (static_cast<std::size_t> (M)
                              * static_cast<std::size_t> (R)
                              * static_cast<std::size_t> (N));
    for (int m = 0; m < M; ++m)
    {
        for (int r = 0; r < R; ++r)
        {
            const float* src = sofaLoader.getHRIR (m, r);
            if (src == nullptr) continue;
            const std::size_t dstOffset = (static_cast<std::size_t> (m)
                                            * static_cast<std::size_t> (R)
                                          + static_cast<std::size_t> (r))
                                         * static_cast<std::size_t> (N);
            std::copy_n (src, N, hrirs.begin() + static_cast<std::ptrdiff_t> (dstOffset));
        }
    }

    const auto result = spatial::solveLSDecoder (dirs.data(), hrirs.data(), M, R, N);
    if (! result.success)
    {
        juce::Logger::writeToLog ("BJF: LS decoder solve failed");
        return;
    }

    lsDecoder.setFilters (result.filters.data(), result.filterLength);
    juce::Logger::writeToLog ("BJF: SOFA loaded — "
                              + juce::String (M) + " directions, "
                              + juce::String (N) + " taps @ "
                              + juce::String (sofaLoader.getSampleRate()) + " Hz");
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
        apvts.getRawParameterValue (pid::waveform)->load());

    voices.setWaveform (wfIndex == 0 ? Oscillator::Waveform::Saw
                                     : Oscillator::Waveform::Square);

    voices.setStackSize (static_cast<int> (
        apvts.getRawParameterValue (pid::stackSize)->load()));
    voices.setDetuneCents (apvts.getRawParameterValue (pid::detuneCents)->load());

    const auto ftIndex = static_cast<int> (
        apvts.getRawParameterValue (pid::filterType)->load());
    voices.setFilterType (ftIndex == 0 ? Filter::Type::LowPass
                          : ftIndex == 1 ? Filter::Type::BandPass
                                         : Filter::Type::HighPass);

    voices.setFilterCutoff    (apvts.getRawParameterValue (pid::filterCutoff)->load());
    voices.setFilterResonance (apvts.getRawParameterValue (pid::filterReso)->load());

    voices.setEnvelopeParameters (
        apvts.getRawParameterValue (pid::attack)->load(),
        apvts.getRawParameterValue (pid::decay)->load(),
        apvts.getRawParameterValue (pid::sustain)->load(),
        apvts.getRawParameterValue (pid::release)->load());
}

void BinauralJungleForgeProcessor::pullSpatialParameters()
{
    constexpr float deg2rad = 0.017453292519943295f;
    const auto azDeg       = apvts.getRawParameterValue (pid::spatialAz)->load();
    const auto elDeg       = apvts.getRawParameterValue (pid::spatialEl)->load();
    const auto spreadAzDeg = apvts.getRawParameterValue (pid::spatialSpreadAz)->load();
    const auto spreadElDeg = apvts.getRawParameterValue (pid::spatialSpreadEl)->load();
    voices.setSpatial (azDeg * deg2rad,       elDeg * deg2rad,
                       spreadAzDeg * deg2rad, spreadElDeg * deg2rad);

    earlyReflections.setRoomSize    (apvts.getRawParameterValue (pid::erRoomSize)->load());
    earlyReflections.setWallDamping (apvts.getRawParameterValue (pid::erWallDamping)->load());
    earlyReflections.setMix         (apvts.getRawParameterValue (pid::erMix)->load());
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

    // Merge any notes the GUI keyboard has pressed into the host MIDI buffer
    // before consuming events. processNextMidiBuffer is the sanctioned bridge
    // between MidiKeyboardComponent (message thread) and the audio thread.
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    pullParametersToVoices();
    pullSpatialParameters();

    const auto gainLin = juce::Decibels::decibelsToGain (
        apvts.getRawParameterValue (pid::gain)->load());

    // Constant-power pan: at p = 0 both gains are 1/sqrt(2); the master gain
    // stage above is the only volume adjustment, so the pan law just
    // redistributes energy between channels without changing perceived level.
    const auto panNorm = juce::jlimit (-1.0f, 1.0f,
        apvts.getRawParameterValue (pid::pan)->load());
    const auto panAngle = (panNorm + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
    const auto panL = std::cos (panAngle);
    const auto panR = std::sin (panAngle);

    float hoaSample[spatial::kNumHoaChannels];

    auto* outL = buffer.getWritePointer (0);
    auto* outR = (numChannels > 1) ? buffer.getWritePointer (1) : nullptr;

    const bool useHRTF = lsDecoder.isReady();

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
            voices.renderNextHoaSample (hoaSample);
            earlyReflections.processSample (hoaSample, hoaSample);

            float l = 0.0f, r = 0.0f;
            if (useHRTF)
                lsDecoder.decodeSample        (hoaSample, l, r);
            else
                fallbackDecoder.decodeSample  (hoaSample, l, r);

            l *= gainLin * panL;
            r *= gainLin * panR;

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

    // Per-block peak update for the meter. Single writer (audio thread),
    // single reader (UI timer) — relaxed atomics are sufficient.
    float peakL = 0.0f, peakR = 0.0f;
    if (outL != nullptr) peakL = buffer.getMagnitude (0, 0, numSamples);
    if (outR != nullptr) peakR = buffer.getMagnitude (1, 0, numSamples);
    outputPeakL.store (peakL, std::memory_order_relaxed);
    outputPeakR.store (peakR, std::memory_order_relaxed);
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
