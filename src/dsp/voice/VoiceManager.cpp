#include "VoiceManager.h"

namespace bjf
{

void VoiceManager::prepare (double sampleRate)
{
    for (auto& v : voices)
        v.prepare (sampleRate);

    noteOnOrder.fill (0);
    orderCounter = 0;
}

std::size_t VoiceManager::findFreeOrOldestVoice() noexcept
{
    // Prefer a fully-released voice.
    for (std::size_t i = 0; i < voices.size(); ++i)
        if (! voices[i].isActive())
            return i;

    // No free voice — steal the one with the smallest note-on stamp.
    std::size_t oldest = 0;
    for (std::size_t i = 1; i < noteOnOrder.size(); ++i)
        if (noteOnOrder[i] < noteOnOrder[oldest])
            oldest = i;
    return oldest;
}

void VoiceManager::noteOn (int midiNoteNumber, float velocity)
{
    const auto idx = findFreeOrOldestVoice();
    voices[idx].noteOn (midiNoteNumber, velocity);
    noteOnOrder[idx] = ++orderCounter;
}

void VoiceManager::noteOff (int midiNoteNumber)
{
    for (auto& v : voices)
        v.noteOff (midiNoteNumber);
}

void VoiceManager::allNotesOff()
{
    for (auto& v : voices)
        v.allNotesOff();
}

void VoiceManager::setWaveform (Oscillator::Waveform w) noexcept
{
    for (auto& v : voices)
        v.setWaveform (w);
}

void VoiceManager::setEnvelopeParameters (float a, float d, float s, float r)
{
    for (auto& v : voices)
        v.setEnvelopeParameters (a, d, s, r);
}

float VoiceManager::renderNextSample() noexcept
{
    float sum = 0.0f;
    for (auto& v : voices)
        sum += v.renderNextSample();
    return sum;
}

int VoiceManager::getActiveVoiceCount() const noexcept
{
    int count = 0;
    for (const auto& v : voices)
        if (v.isActive())
            ++count;
    return count;
}

} // namespace bjf
