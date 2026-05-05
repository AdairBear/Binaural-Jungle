#pragma once

#include <array>

#include "Oscillator.h"

namespace bjf
{

// Detuned multi-oscillator stack — the engine behind the Juno/Jupiter "lush"
// pad sound. 1..kMaxOscs oscillators run at the base frequency offset by an
// evenly-distributed detune in cents (symmetric around 0), with scattered
// initial phases so unisons sum to a thick chorus rather than a synchronised
// single transient.
//
// Output is normalised by sqrt(N) so perceived level stays roughly constant
// as the stack size changes.
class OscillatorStack
{
public:
    static constexpr int kMaxOscs = 7;

    void setSampleRate (double sampleRate) noexcept;
    void setBaseFrequency (float hz) noexcept;
    void setStackSize (int count) noexcept;          // clamped to [1, kMaxOscs]
    void setDetuneCents (float totalSpread) noexcept; // half-spread on each side; 0 = unison
    void setWaveform (Oscillator::Waveform w) noexcept;
    void resetPhases() noexcept;

    int   getStackSize() const noexcept { return stackSize; }
    float getDetuneCents() const noexcept { return detuneCents; }

    float renderNextSample() noexcept;

private:
    void recomputeFrequencies() noexcept;

    std::array<Oscillator, kMaxOscs> oscillators {};
    int   stackSize    = 3;
    float baseHz       = 440.0f;
    float detuneCents  = 7.0f;
};

} // namespace bjf
