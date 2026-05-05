// Smoke test for step-7 per-voice HOA encoding. Verifies that the voice
// pool, when summed in the HOA domain, behaves the way the spread parameter
// promises:
//   - spread=0 collapses every voice to the centre, so every sample on the
//     bus reads as a single mono source at that position (Y = sin(az) · W,
//     X = cos(az) · W exactly).
//   - spread = full ring with N voices distributes them around the listener;
//     odd-symmetry channels (Y, X, etc.) cancel and the result is dominated
//     by the omni W channel.
//   - centre azimuth steers the bus (collapsed-to-centre case), confirming
//     the spread parameters compose with the centre parameters correctly.

#include "../src/dsp/spatial/HOACoefficients.h"
#include "../src/dsp/voice/VoiceManager.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr float kTwoPi  = 6.283185307179586f;
constexpr float kHalfPi = 1.5707963267948966f;

// Maxima we care about for verifying the spread behaviour.
struct Stats
{
    float maxAbsW       = 0.0f;
    float maxAbsY       = 0.0f;
    float maxAbsX       = 0.0f;
    // Per-sample residual: |Y - sin(az)·W|, |X - cos(az)·W|. With every voice
    // collapsed at (az, 0), these must be ~0 every sample because each voice
    // contributes the same linear combination.
    float maxResidualY  = 0.0f;
    float maxResidualX  = 0.0f;
};

Stats analyse (bjf::VoiceManager& vm, float az, int numSamples)
{
    Stats stats {};
    const float sinAz = std::sin (az);
    const float cosAz = std::cos (az);

    float bus[bjf::spatial::kNumHoaChannels];
    for (int n = 0; n < numSamples; ++n)
    {
        vm.renderNextHoaSample (bus);
        stats.maxAbsW      = std::max (stats.maxAbsW, std::abs (bus[0]));
        stats.maxAbsY      = std::max (stats.maxAbsY, std::abs (bus[1]));
        stats.maxAbsX      = std::max (stats.maxAbsX, std::abs (bus[3]));
        stats.maxResidualY = std::max (stats.maxResidualY,
                                       std::abs (bus[1] - sinAz * bus[0]));
        stats.maxResidualX = std::max (stats.maxResidualX,
                                       std::abs (bus[3] - cosAz * bus[0]));
    }
    return stats;
}

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }
}

int main()
{
    constexpr double sampleRate = 48000.0;
    constexpr int    numSamples = 2400; // 50 ms — past the attack, into sustain
    const     int    chord[]    = { 60, 64, 67, 71, 74 };

    // ── Case 1: spread=0 with centre at the left ear (+90° az). Every
    //    voice sits on top of the others, so the bus must equal a single
    //    mono source at +π/2 sample-for-sample. ──
    {
        bjf::VoiceManager vm;
        vm.prepare (sampleRate);
        vm.setWaveform (bjf::Oscillator::Waveform::Saw);
        vm.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);
        vm.setSpatial (kHalfPi, 0.0f, 0.0f, 0.0f);
        for (auto n : chord) vm.noteOn (n, 1.0f);

        const auto s = analyse (vm, kHalfPi, numSamples);
        std::cout << "spread=0 @ +90°: |W|max=" << s.maxAbsW
                  << "  |Y|max=" << s.maxAbsY
                  << "  |X|max=" << s.maxAbsX
                  << "  resY=" << s.maxResidualY
                  << "  resX=" << s.maxResidualX << '\n';

        if (s.maxAbsW < 0.1f)
            return fail ("collapsed bus too quiet — voices not summing in HOA");

        // sin(+π/2) = 1 → Y must track W exactly.
        if (s.maxResidualY > 1e-4f)
            return fail ("collapsed @ +90°: Y must equal sin(az)·W sample-by-sample");

        // cos(+π/2) = 0 → X must be ≈0 every sample.
        if (s.maxResidualX > 1e-4f)
            return fail ("collapsed @ +90°: X must equal cos(az)·W sample-by-sample");
        if (s.maxAbsX > 1e-4f)
            return fail ("collapsed @ +90°: X (front-back) should be ~0");
    }

    // ── Case 2: spread = full ring around the front. Fire the SAME note
    //    on all 16 slots so each voice produces an identical mono signal;
    //    that turns the ring sum into an algebraic identity:
    //        Y(t) = v(t) · Σ_i sin(az_i)
    //    With evenly-spaced az_i around a full circle, Σ sin = Σ cos = 0,
    //    so Y, X, and every other odd-symmetric SH channel cancel sample-
    //    for-sample. (Mixed notes wouldn't cancel — different frequencies
    //    decorrelate the voices and the ring symmetry only gates the mean,
    //    not the instantaneous sum.) ──
    {
        bjf::VoiceManager ring;
        ring.prepare (sampleRate);
        ring.setWaveform (bjf::Oscillator::Waveform::Saw);
        ring.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);
        ring.setSpatial (0.0f, 0.0f, kTwoPi, 0.0f);
        for (int i = 0; i < bjf::VoiceManager::kMaxVoices; ++i)
            ring.noteOn (60, 1.0f);

        const auto s = analyse (ring, 0.0f, numSamples);
        const auto yRatio = s.maxAbsY / std::max (s.maxAbsW, 1e-9f);
        const auto xRatio = s.maxAbsX / std::max (s.maxAbsW, 1e-9f);
        std::cout << "spread=2π     : |W|max=" << s.maxAbsW
                  << "  |Y|max=" << s.maxAbsY
                  << "  |X|max=" << s.maxAbsX
                  << "  |Y/W|=" << yRatio
                  << "  |X/W|=" << xRatio << '\n';

        if (s.maxAbsW < 0.1f)
            return fail ("ring-distributed bus too quiet");
        if (yRatio > 1e-4f)
            return fail ("ring: Y should cancel sample-for-sample");
        if (xRatio > 1e-4f)
            return fail ("ring: X should cancel sample-for-sample");
    }

    // ── Case 3: collapsed centre at the right ear (-90°). Y must flip sign
    //    relative to W: every sample Y = -W. ──
    {
        bjf::VoiceManager vm;
        vm.prepare (sampleRate);
        vm.setWaveform (bjf::Oscillator::Waveform::Saw);
        vm.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);
        vm.setSpatial (-kHalfPi, 0.0f, 0.0f, 0.0f);
        for (auto n : chord) vm.noteOn (n, 1.0f);

        const auto s = analyse (vm, -kHalfPi, numSamples);
        std::cout << "spread=0 @ -90°: |W|max=" << s.maxAbsW
                  << "  |Y|max=" << s.maxAbsY
                  << "  resY=" << s.maxResidualY << '\n';

        if (s.maxAbsW < 0.1f)
            return fail ("right-collapsed bus too quiet");
        if (s.maxResidualY > 1e-4f)
            return fail ("right collapsed @ -90°: Y must equal -W sample-by-sample");
    }

    // ── Case 4: spread but only one voice playing → the lone voice still
    //    sits at slot-0's spatial position, NOT at the centre. The bus must
    //    reflect slot 0's offset. With spread = 2π and centre = 0, slot 0
    //    is at (0.5/16 - 0.5)·2π = (-15/16)·π ≈ -2.945 rad. The Y and X
    //    channels follow that azimuth, not the centre. ──
    {
        bjf::VoiceManager vm;
        vm.prepare (sampleRate);
        vm.setWaveform (bjf::Oscillator::Waveform::Saw);
        vm.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);
        vm.setSpatial (0.0f, 0.0f, kTwoPi, 0.0f);
        vm.noteOn (60, 1.0f);

        constexpr int N = bjf::VoiceManager::kMaxVoices;
        const float t0  = (0.5f / static_cast<float> (N)) - 0.5f;
        const float az0 = kTwoPi * t0;

        const auto s = analyse (vm, az0, numSamples);
        std::cout << "single voice slot 0 (az=" << az0 << "): "
                  << "resY=" << s.maxResidualY
                  << "  resX=" << s.maxResidualX << '\n';

        if (s.maxAbsW < 0.05f)
            return fail ("single-voice bus too quiet");
        if (s.maxResidualY > 1e-4f)
            return fail ("single voice: Y must follow slot-0 azimuth");
        if (s.maxResidualX > 1e-4f)
            return fail ("single voice: X must follow slot-0 azimuth");
    }

    std::cout << "PASS\n";
    return 0;
}
