// Smoke test for bjf::Voice. Renders 100ms after a MIDI note-on and asserts
// that audible signal is produced. Run after every build to catch regressions
// in the audio path before loading in a DAW.

#include "../src/dsp/voice/Voice.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
struct Stats { float peak; double rms; int nonZeroCount; };

Stats render (bjf::Voice& v, int numSamples)
{
    Stats s {};
    double sumSq = 0.0;
    for (int i = 0; i < numSamples; ++i)
    {
        const auto x = v.renderNextSample();
        s.peak = std::max (s.peak, std::abs (x));
        sumSq += static_cast<double> (x) * x;
        if (std::abs (x) > 1e-6f) ++s.nonZeroCount;
    }
    s.rms = std::sqrt (sumSq / numSamples);
    return s;
}

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }
}

int main()
{
    constexpr double sampleRate = 48000.0;
    constexpr int    numSamples = 4800; // 100 ms

    bjf::Voice voice;
    voice.prepare (sampleRate);
    voice.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);

    // ── Saw, MIDI 60 (middle C, ~261.63 Hz) ──
    voice.setWaveform (bjf::Oscillator::Waveform::Saw);
    voice.noteOn (60, 1.0f);
    const auto saw = render (voice, numSamples);

    std::cout << "saw  : peak=" << saw.peak
              << "  rms=" << saw.rms
              << "  nonzero=" << saw.nonZeroCount << "/" << numSamples << '\n';

    if (saw.peak < 0.1f)        return fail ("saw peak too low — voice is silent");
    if (saw.peak > 1.01f)       return fail ("saw peak > 1.0 — clipping risk");
    if (saw.nonZeroCount < numSamples / 2)
        return fail ("saw is silent for more than half the buffer");

    // ── Note-off → envelope releases to silence ──
    voice.noteOff (60);
    const auto tail = render (voice, static_cast<int> (sampleRate * 0.5)); // 500 ms tail
    std::cout << "tail : peak=" << tail.peak << "  rms=" << tail.rms << '\n';

    if (voice.isActive())       return fail ("voice still active after note-off + 500 ms");

    // ── Square waveform ──
    bjf::Voice voice2;
    voice2.prepare (sampleRate);
    voice2.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);
    voice2.setWaveform (bjf::Oscillator::Waveform::Square);
    voice2.noteOn (69, 1.0f); // A4 = 440 Hz
    const auto sq = render (voice2, numSamples);

    std::cout << "sq   : peak=" << sq.peak
              << "  rms=" << sq.rms
              << "  nonzero=" << sq.nonZeroCount << "/" << numSamples << '\n';

    if (sq.peak < 0.5f)         return fail ("square peak too low");
    if (sq.nonZeroCount < numSamples - 100)
        return fail ("square has too many zero-crossings — phase math broken");

    std::cout << "PASS\n";
    return 0;
}
