#pragma once

namespace bjf
{

// Naive (non-band-limited) oscillator. Aliases at high frequencies — fine for
// step-2 smoke-test; replace with PolyBLEP / band-limited generator when we
// build OscillatorStack in step 4.
class Oscillator
{
public:
    enum class Waveform { Saw, Square };

    void setSampleRate (double sampleRate) noexcept;
    void setFrequency (float hz) noexcept;
    void setWaveform (Waveform w) noexcept;
    void resetPhase() noexcept;

    float renderNextSample() noexcept;

private:
    double phase = 0.0;
    double phaseInc = 0.0;
    double sampleRateHz = 44100.0;
    Waveform waveform = Waveform::Saw;
};

} // namespace bjf
