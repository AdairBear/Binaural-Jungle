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
    void setEnvelopeParameters (float attackSec, float decaySec,
                                float sustainLevel, float releaseSec);

    // Sum of all active voices, mono. Caller is responsible for headroom —
    // 16 voices summed straight can exceed unity; the master gain stage
    // attenuates downstream.
    float renderNextSample() noexcept;

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
