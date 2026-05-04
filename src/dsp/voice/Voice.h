#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "Oscillator.h"

namespace bjf
{

// Single mono voice: one oscillator + amp ADSR. Last-note-priority for
// monophonic MIDI. Polyphonic VoiceManager arrives in step 3.
class Voice
{
public:
    void prepare (double sampleRate);

    void noteOn (int midiNoteNumber, float velocity);
    void noteOff (int midiNoteNumber);
    void allNotesOff();

    bool isActive() const noexcept;

    void setWaveform (Oscillator::Waveform w) noexcept;
    void setEnvelopeParameters (float attackSec, float decaySec,
                                float sustainLevel, float releaseSec);

    float renderNextSample() noexcept;

private:
    Oscillator oscillator;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams { 0.01f, 0.2f, 0.7f, 0.5f };

    int currentNote = -1;
    float velocityGain = 1.0f;
};

} // namespace bjf
