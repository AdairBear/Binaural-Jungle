// End-to-end smoke test for the LS HOA→binaural decoder driven by real
// MIT KEMAR HRTFs. Builds the full pipeline:
//   SOFALoader → solveLSDecoder → BinauralLSDecoder
// and drives encoded impulses through it to verify ipsilateral dominance.

#include "../src/dsp/spatial/BinauralLSDecoder.h"
#include "../src/dsp/spatial/DecoderMatrix.h"
#include "../src/dsp/spatial/HOAEncoder.h"
#include "../src/dsp/spatial/SOFALoader.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#ifndef BJF_DEFAULT_SOFA_PATH
 #error "BJF_DEFAULT_SOFA_PATH not defined — CMake should provide it"
#endif

namespace
{
constexpr float kPi     = 3.141592653589793f;
constexpr float kHalfPi = 1.5707963267948966f;

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }

// Drive a unit impulse encoded at (az, el) through the decoder and return
// the energy of the L and R outputs over a window covering the full filter
// tail.
struct Energies { double L, R; };

Energies impulseEnergy (bjf::spatial::HOAEncoder& enc,
                        bjf::spatial::BinauralLSDecoder& dec,
                        float az, float el, int tailSamples)
{
    enc.setPosition (az, el);
    dec.reset();

    float hoa[bjf::spatial::kNumHoaChannels];

    Energies e {};
    for (int n = 0; n < tailSamples; ++n)
    {
        const float drive = (n == 0) ? 1.0f : 0.0f;
        enc.encodeSample (drive, hoa);
        float l = 0.0f, r = 0.0f;
        dec.decodeSample (hoa, l, r);
        e.L += static_cast<double> (l) * l;
        e.R += static_cast<double> (r) * r;
    }
    return e;
}
}

int main()
{
    bjf::spatial::SOFALoader loader;
    const auto status = loader.loadFromFile (BJF_DEFAULT_SOFA_PATH);
    if (! status.success)
        return fail ("SOFA load failed");

    const int M = loader.getNumDirections();
    const int N = loader.getFilterLength();
    const int R = loader.getNumReceivers();
    std::cout << "SOFA: M=" << M << "  N=" << N << "  R=" << R << '\n';

    // Marshal directions and HRIRs into the contiguous form the solver expects.
    std::vector<float> dirs (static_cast<std::size_t> (M) * 2);
    for (int m = 0; m < M; ++m)
    {
        float az = 0.0f, el = 0.0f;
        loader.getDirection (m, az, el);
        dirs[static_cast<std::size_t> (m) * 2 + 0] = az;
        dirs[static_cast<std::size_t> (m) * 2 + 1] = el;
    }

    std::vector<float> hrirs (static_cast<std::size_t> (M)
                              * static_cast<std::size_t> (R)
                              * static_cast<std::size_t> (N));
    for (int m = 0; m < M; ++m)
    {
        for (int r = 0; r < R; ++r)
        {
            const float* src = loader.getHRIR (m, r);
            const std::size_t dstOffset = (static_cast<std::size_t> (m)
                                            * static_cast<std::size_t> (R)
                                          + static_cast<std::size_t> (r))
                                         * static_cast<std::size_t> (N);
            std::copy_n (src, N, hrirs.begin() + static_cast<std::ptrdiff_t> (dstOffset));
        }
    }

    const auto result = bjf::spatial::solveLSDecoder (dirs.data(), hrirs.data(), M, R, N);
    if (! result.success)
        return fail ("LS solve failed");

    std::cout << "LS solve: " << result.numHoaChannels << " ch × "
              << result.numEars << " ears × "
              << result.filterLength << " taps\n";

    bjf::spatial::HOAEncoder       enc;
    bjf::spatial::BinauralLSDecoder dec;
    dec.prepare (48000.0, 256);
    dec.setFilters (result.filters.data(), result.filterLength);

    const int tail = N + 64; // a bit of slack past filter length

    // ── Source at left ear (+90° az): L energy >> R ──
    const auto leftSrc  = impulseEnergy (enc, dec, +kHalfPi, 0.0f, tail);
    std::cout << "src @ +90° az: L²=" << leftSrc.L << "  R²=" << leftSrc.R << '\n';
    if (leftSrc.L < 4.0 * leftSrc.R)
        return fail ("source at +90° did not produce ipsilateral L dominance ≥ 4×");

    // ── Source at right ear (-90° az): R energy >> L ──
    const auto rightSrc = impulseEnergy (enc, dec, -kHalfPi, 0.0f, tail);
    std::cout << "src @ -90° az: L²=" << rightSrc.L << "  R²=" << rightSrc.R << '\n';
    if (rightSrc.R < 4.0 * rightSrc.L)
        return fail ("source at -90° did not produce ipsilateral R dominance ≥ 4×");

    // ── Source at front: L ≈ R within 50% (HRTFs are roughly symmetric for
    //    median-plane directions; the LS solve preserves that). ──
    const auto front = impulseEnergy (enc, dec, 0.0f, 0.0f, tail);
    std::cout << "src @  front : L²=" << front.L  << "  R²=" << front.R  << '\n';
    if (front.L <= 0.0 || front.R <= 0.0)
        return fail ("front energy not positive in both ears");
    const double ratio = std::max (front.L, front.R) / std::min (front.L, front.R);
    if (ratio > 2.0)
        return fail ("front energy badly asymmetric — HRTF/decoder mismatch?");

    // ── Sweep the lateral hemisphere: ipsilateral always dominates at ±60..120°. ──
    for (float deg : { -120.0f, -90.0f, -60.0f, 60.0f, 90.0f, 120.0f })
    {
        const auto az = deg * kPi / 180.0f;
        const auto e  = impulseEnergy (enc, dec, az, 0.0f, tail);
        if (deg > 0.0f && e.L <= e.R)
            return fail ("left lateral hemisphere: L should dominate");
        if (deg < 0.0f && e.R <= e.L)
            return fail ("right lateral hemisphere: R should dominate");
    }

    std::cout << "PASS\n";
    return 0;
}
