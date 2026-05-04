#include "Voice.h"

namespace bjf
{

void Voice::prepare (double sampleRate)
{
    oscillator.setSampleRate (sampleRate);
    adsr.setSampleRate (sampleRate);
    adsr.setParameters (adsrParams);
}

void Voice::noteOn (int midiNoteNumber, float velocity)
{
    currentNote = midiNoteNumber;
    velocityGain = velocity;

    const auto hz = static_cast<float> (
        juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    oscillator.setFrequency (hz);
    oscillator.resetPhase();

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

void Voice::setWaveform (Oscillator::Waveform w) noexcept
{
    oscillator.setWaveform (w);
}

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
    return oscillator.renderNextSample() * env * velocityGain;
}

} // namespace bjf
