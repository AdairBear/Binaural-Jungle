#include "Voice.h"

namespace bjf
{

void Voice::prepare (double sampleRate)
{
    oscStack.setSampleRate (sampleRate);
    filter.prepare (sampleRate);
    adsr.setSampleRate (sampleRate);
    adsr.setParameters (adsrParams);
}

void Voice::noteOn (int midiNoteNumber, float velocity)
{
    currentNote   = midiNoteNumber;
    velocityGain  = velocity;

    const auto hz = static_cast<float> (
        juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    oscStack.setBaseFrequency (hz);
    oscStack.resetPhases();
    filter.reset();

    adsr.noteOn();
}

void Voice::noteOff (int midiNoteNumber)
{
    // Last-note-wins: only release if this matches the currently sounding note.
    // A second note already retriggered the voice and "owns" it now.
    if (midiNoteNumber == currentNote)
    {
        adsr.noteOff();
        currentNote = -1;
    }
}

void Voice::allNotesOff()
{
    adsr.noteOff();
    currentNote = -1;
}

bool Voice::isActive() const noexcept
{
    return adsr.isActive();
}

void Voice::setWaveform (Oscillator::Waveform w) noexcept     { oscStack.setWaveform (w); }
void Voice::setStackSize (int count) noexcept                 { oscStack.setStackSize (count); }
void Voice::setDetuneCents (float cents) noexcept             { oscStack.setDetuneCents (cents); }

void Voice::setFilterType (Filter::Type t) noexcept           { filter.setType (t); }
void Voice::setFilterCutoff (float hz) noexcept               { filter.setCutoff (hz); }
void Voice::setFilterResonance (float q) noexcept             { filter.setResonance (q); }

void Voice::setEnvelopeParameters (float a, float d, float s, float r)
{
    adsrParams.attack  = a;
    adsrParams.decay   = d;
    adsrParams.sustain = s;
    adsrParams.release = r;
    adsr.setParameters (adsrParams);
}

float Voice::renderNextSample() noexcept
{
    if (! adsr.isActive())
        return 0.0f;

    const auto env = adsr.getNextSample();
    const auto raw = oscStack.renderNextSample();
    return filter.process (raw) * env * velocityGain;
}

} // namespace bjf
