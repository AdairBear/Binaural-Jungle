// Smoke test for bjf::Voice. Renders 100ms after a MIDI note-on and asserts
// that audible signal is produced. Step 4 adds detuned-stack + SVF coverage:
// a wide-open filter passes the stack, a tight LP attenuates a high note.

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

    // ── Saw, MIDI 60, 3-osc stack with 7c detune, wide-open LP. ──
    bjf::Voice voice;
    voice.prepare (sampleRate);
    voice.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);
    voice.setWaveform (bjf::Oscillator::Waveform::Saw);
    voice.setStackSize (3);
    voice.setDetuneCents (7.0f);
    voice.setFilterType (bjf::Filter::Type::LowPass);
    voice.setFilterCutoff (20000.0f);
    voice.setFilterResonance (0.707f);
    voice.noteOn (60, 1.0f);
    const auto saw = render (voice, numSamples);

    std::cout << "saw  : peak=" << saw.peak
              << "  rms=" << saw.rms
              << "  nonzero=" << saw.nonZeroCount << "/" << numSamples << '\n';

    if (saw.peak < 0.1f)        return fail ("saw peak too low — voice is silent");
    if (saw.peak > 1.5f)        return fail ("saw peak way too hot — normalisation broken");
    if (saw.nonZeroCount < numSamples / 2)
        return fail ("saw is silent for more than half the buffer");

    // ── Note-off → envelope releases to silence ──
    voice.noteOff (60);
    const auto tail = render (voice, static_cast<int> (sampleRate * 0.5)); // 500 ms tail
    std::cout << "tail : peak=" << tail.peak << "  rms=" << tail.rms << '\n';
    if (voice.isActive())       return fail ("voice still active after note-off + 500 ms");

    // ── Square waveform, single-osc stack ──
    bjf::Voice voice2;
    voice2.prepare (sampleRate);
    voice2.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);
    voice2.setWaveform (bjf::Oscillator::Waveform::Square);
    voice2.setStackSize (1);
    voice2.setDetuneCents (0.0f);
    voice2.setFilterType (bjf::Filter::Type::LowPass);
    voice2.setFilterCutoff (20000.0f);
    voice2.setFilterResonance (0.707f);
    voice2.noteOn (69, 1.0f); // A4 = 440 Hz
    const auto sq = render (voice2, numSamples);

    std::cout << "sq   : peak=" << sq.peak
              << "  rms=" << sq.rms
              << "  nonzero=" << sq.nonZeroCount << "/" << numSamples << '\n';

    if (sq.peak < 0.5f)         return fail ("square peak too low");
    if (sq.nonZeroCount < numSamples - 100)
        return fail ("square has too many zero-crossings — phase math broken");

    // ── Tight LP cutoff strongly attenuates a high note. Compare wide vs
    //    tight on the same waveform / note / stack. ──
    auto runLp = [&](float cutoffHz)
    {
        bjf::Voice v;
        v.prepare (sampleRate);
        v.setEnvelopeParameters (0.001f, 0.5f, 1.0f, 0.1f); // hold sustain at unity
        v.setWaveform (bjf::Oscillator::Waveform::Saw);
        v.setStackSize (3);
        v.setDetuneCents (10.0f);
        v.setFilterType (bjf::Filter::Type::LowPass);
        v.setFilterCutoff (cutoffHz);
        v.setFilterResonance (0.707f);
        v.noteOn (84, 1.0f); // C6, base ~1046 Hz, rich harmonics above
        // skip 200 ms of attack/decay to land on sustain
        render (v, static_cast<int> (sampleRate * 0.2));
        return render (v, numSamples);
    };

    const auto lpOpen  = runLp (18000.0f);
    const auto lpTight = runLp (300.0f);

    std::cout << "LP open  @18k: rms=" << lpOpen.rms  << "  peak=" << lpOpen.peak  << '\n';
    std::cout << "LP tight @300: rms=" << lpTight.rms << "  peak=" << lpTight.peak << '\n';

    if (lpTight.rms >= lpOpen.rms)
        return fail ("tight LP did not attenuate the high saw — filter not wired into voice");
    if (lpTight.rms > lpOpen.rms * 0.6)
        return fail ("tight LP barely attenuated — filter routing weak");

    std::cout << "PASS\n";
    return 0;
}
