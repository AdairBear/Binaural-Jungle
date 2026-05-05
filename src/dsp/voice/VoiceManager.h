#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Voice.h"

namespace bjf
{

// Polyphonic 16-voice manager. On note-on, picks a free voice if one is
// available, otherwise steals the voice that has been sounding the longest.
// Note-offs are routed by MIDI note number — Voice::noteOff already ignores
// numbers it isn't currently sounding, so a broadcast is correct and cheap.
//
// MPE awareness, release-tail-priority stealing, and per-voice spatial state
// are deferred to later steps (see design/01-architecture.md).
class VoiceManager
{
public:
    static constexpr int kMaxVoices = 16;

    void prepare (double sampleRate);

    void noteOn (int midiNoteNumber, float velocity);
    void noteOff (int midiNoteNumber);
    void allNotesOff();

    void setWaveform (Oscillator::Waveform w) noexcept;
    void setStackSize (int count) noexcept;
    void setDetuneCents (float cents) noexcept;

    void setFilterType (Filter::Type t) noexcept;
    void setFilterCutoff (float hz) noexcept;
    void setFilterResonance (float q) noexcept;

    void setEnvelopeParameters (float attackSec, float decaySec,
                                float sustainLevel, float releaseSec);

    // Distributes the voice pool around `(centerAz, centerEl)` covering an
    // angular range of `spreadAz` × `spreadEl` (all radians). Each voice
    // slot gets a deterministic position, so noteOn/noteOff don't scramble
    // the spatial layout — slot i is always at the same offset relative to
    // center. spreadAz = 2π gives a full ring; 0 collapses every voice to
    // the centre. The (i + 0.5)/N quantisation prevents endpoints from
    // colliding when the spread reaches a full circle.
    void setSpatial (float centerAzRad, float centerElRad,
                     float spreadAzRad, float spreadElRad) noexcept;

    // Sum of all active voices, mono. Caller is responsible for headroom —
    // 16 voices summed straight can exceed unity; the master gain stage
    // attenuates downstream.
    float renderNextSample() noexcept;

    // Sum of all active voices in the HOA domain (16 channels, ACN/SN3D).
    // Each voice encodes itself at its own (az, el) before summing, so the
    // resulting bus carries the full spread of the polyphonic image.
    void renderNextHoaSample (float* hoa16) noexcept;

    int getActiveVoiceCount() const noexcept;

private:
    std::size_t findFreeOrOldestVoice() noexcept;

    std::array<Voice, kMaxVoices> voices {};

    // Monotonic note-on counter; per-voice age stamp. Lowest stamp = oldest
    // currently allocated voice, which is the steal candidate.
    std::array<std::uint64_t, kMaxVoices> noteOnOrder {};
    std::uint64_t orderCounter = 0;
};

} // namespace bjf
