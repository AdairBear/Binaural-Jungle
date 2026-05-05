#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "Filter.h"
#include "Oscillator.h"
#include "OscillatorStack.h"

namespace bjf
{

// Single mono voice — detuned oscillator stack → SVF → amp ADSR.
// Polyphonic dispatch happens in VoiceManager; per-voice spatial position
// (HOA encode) lands in step 5+.
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

    float renderNextSample() noexcept;

private:
    OscillatorStack oscStack;
    Filter filter;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams { 0.01f, 0.2f, 0.7f, 0.5f };

    int currentNote = -1;
    float velocityGain = 1.0f;
};

} // namespace bjf
