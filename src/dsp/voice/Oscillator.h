#pragma once

namespace bjf
{

// Naive (non-band-limited) oscillator. Aliases at high frequencies — acceptable
// for the v1 prototype; PolyBLEP / band-limited generation is a follow-up.
// Used as a single voice in step 2 and as a unit cell of OscillatorStack from
// step 4 onward.
class Oscillator
{
public:
    enum class Waveform { Saw, Square };

    void setSampleRate (double sampleRate) noexcept;
    void setFrequency (float hz) noexcept;
    void setWaveform (Waveform w) noexcept;
    void resetPhase() noexcept;
    void setPhase (float phase01) noexcept;

    float renderNextSample() noexcept;

private:
    double phase = 0.0;
    double phaseInc = 0.0;
    double sampleRateHz = 44100.0;
    Waveform waveform = Waveform::Saw;
};

} // namespace bjf
