#include "VoiceManager.h"

#include "../spatial/HOACoefficients.h"

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

void VoiceManager::setStackSize (int count) noexcept
{
    for (auto& v : voices)
        v.setStackSize (count);
}

void VoiceManager::setDetuneCents (float cents) noexcept
{
    for (auto& v : voices)
        v.setDetuneCents (cents);
}

void VoiceManager::setFilterType (Filter::Type t) noexcept
{
    for (auto& v : voices)
        v.setFilterType (t);
}

void VoiceManager::setFilterCutoff (float hz) noexcept
{
    for (auto& v : voices)
        v.setFilterCutoff (hz);
}

void VoiceManager::setFilterResonance (float q) noexcept
{
    for (auto& v : voices)
        v.setFilterResonance (q);
}

void VoiceManager::setEnvelopeParameters (float a, float d, float s, float r)
{
    for (auto& v : voices)
        v.setEnvelopeParameters (a, d, s, r);
}

void VoiceManager::setSpatial (float centerAzRad, float centerElRad,
                               float spreadAzRad, float spreadElRad) noexcept
{
    constexpr float invN = 1.0f / static_cast<float> (kMaxVoices);
    for (int i = 0; i < kMaxVoices; ++i)
    {
        // t ∈ (-0.5, +0.5), evenly spaced with no endpoint collision when
        // spread covers a full circle.
        const float t  = (static_cast<float> (i) + 0.5f) * invN - 0.5f;
        const float az = centerAzRad + spreadAzRad * t;
        const float el = centerElRad + spreadElRad * t;
        voices[static_cast<std::size_t> (i)].setSpatialPosition (az, el);
    }
}

float VoiceManager::renderNextSample() noexcept
{
    float sum = 0.0f;
    for (auto& v : voices)
        sum += v.renderNextSample();
    return sum;
}

void VoiceManager::renderNextHoaSample (float* hoa16) noexcept
{
    for (int c = 0; c < spatial::kNumHoaChannels; ++c)
        hoa16[c] = 0.0f;

    for (auto& v : voices)
        v.addNextHoaSample (hoa16);
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
