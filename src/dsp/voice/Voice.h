#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "../spatial/HOAEncoder.h"
#include "Filter.h"
#include "Oscillator.h"
#include "OscillatorStack.h"

namespace bjf
{

// Single mono voice — detuned oscillator stack → SVF → amp ADSR, plus a
// per-voice HOA encoder so each voice can sit at its own position in the
// spatial field. The encoder is owned here rather than shared at the
// processor level because the signature spread effect (step 7) needs each
// polyphonic voice to occupy a different point in the 3D field; downstream
// the HOA buses are summed and decoded once.
class Voice
{
public:
    void prepare (double sampleRate);

    void noteOn (int midiNoteNumber, float velocity);
    void noteOff (int midiNoteNumber);
    void allNotesOff();

    bool isActive() const noexcept;

    // Oscillator-stack settings
    void setWaveform (Oscillator::Waveform w) noexcept;
    void setStackSize (int count) noexcept;
    void setDetuneCents (float cents) noexcept;

    // Filter settings
    void setFilterType (Filter::Type t) noexcept;
    void setFilterCutoff (float hz) noexcept;
    void setFilterResonance (float q) noexcept;

    // Amp envelope
    void setEnvelopeParameters (float attackSec, float decaySec,
                                float sustainLevel, float releaseSec);

    // Spatial position in radians. Used by the per-voice HOA encoder.
    void setSpatialPosition (float azRad, float elRad) noexcept;

    float renderNextSample() noexcept;

    // Adds this voice's HOA-encoded sample into `hoa16` (does NOT clear).
    // Silent voices return without touching the buffer, so the caller can
    // sum across the voice pool with no extra branch.
    void addNextHoaSample (float* hoa16) noexcept;

private:
    OscillatorStack oscStack;
    Filter filter;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams { 0.01f, 0.2f, 0.7f, 0.5f };
    spatial::HOAEncoder encoder;

    int currentNote = -1;
    float velocityGain = 1.0f;
};

} // namespace bjf
