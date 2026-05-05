// Smoke test for the step-5 spatial pipeline: encode a unity mono source at
// a series of azimuths, decode through the prototype 2-speaker sampling
// decoder, and verify the result rotates around the listener as expected.
//
// Acceptance criteria for the prototype:
//   - source at left ear (az=+90°)  → L >> R
//   - source at right ear (az=-90°) → R >> L
//   - source at front (az=0°)       → L ≈ R, both audible
//   - left ear position decodes to L=1.0 exactly (analytic test of scale)

#include "../src/dsp/spatial/HOAEncoder.h"
#include "../src/dsp/spatial/SimpleStereoDecoder.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr float kPi     = 3.141592653589793f;
constexpr float kHalfPi = 1.5707963267948966f;

struct StereoSample { float l, r; };

StereoSample encodeAndDecode (bjf::spatial::HOAEncoder& enc,
                              const bjf::spatial::SimpleStereoDecoder& dec,
                              float az, float el, float mono)
{
    enc.setPosition (az, el);
    float hoa[bjf::spatial::kNumHoaChannels];
    enc.encodeSample (mono, hoa);
    StereoSample s {};
    dec.decodeSample (hoa, s.l, s.r);
    return s;
}

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }
}

int main()
{
    bjf::spatial::HOAEncoder           enc;
    bjf::spatial::SimpleStereoDecoder  dec;

    // ── Source aimed straight at the left ear: L should be exactly 1.0,
    //    R should be exactly 0.0 (orthogonal columns at order 3). ──
    const auto leftEar = encodeAndDecode (enc, dec, +kHalfPi, 0.0f, 1.0f);
    std::cout << "source @ left  (+90°): L=" << leftEar.l << "  R=" << leftEar.r << '\n';
    if (std::abs (leftEar.l - 1.0f) > 1e-4f) return fail ("source at left ear: L != 1.0");
    if (std::abs (leftEar.r)        > 1e-4f) return fail ("source at left ear: R != 0.0");

    const auto rightEar = encodeAndDecode (enc, dec, -kHalfPi, 0.0f, 1.0f);
    std::cout << "source @ right (-90°): L=" << rightEar.l << "  R=" << rightEar.r << '\n';
    if (std::abs (rightEar.l)        > 1e-4f) return fail ("source at right ear: L != 0.0");
    if (std::abs (rightEar.r - 1.0f) > 1e-4f) return fail ("source at right ear: R != 1.0");

    // ── Source in front: L ≈ R, both > 0 (front sounds emerge from both ears
    //    in the prototype; level is lower than ipsilateral because we only
    //    have 2 virtual speakers). ──
    const auto front = encodeAndDecode (enc, dec, 0.0f, 0.0f, 1.0f);
    std::cout << "source @ front    : L=" << front.l    << "  R=" << front.r    << '\n';
    if (std::abs (front.l - front.r) > 1e-4f)
        return fail ("front: L should equal R by symmetry");
    if (front.l <= 0.0f) return fail ("front: L not positive");
    if (front.r <= 0.0f) return fail ("front: R not positive");

    // ── Source behind: also symmetric, also positive (or at least finite). ──
    const auto back = encodeAndDecode (enc, dec, kPi, 0.0f, 1.0f);
    std::cout << "source @ back     : L=" << back.l     << "  R=" << back.r     << '\n';
    if (std::abs (back.l - back.r) > 1e-4f)
        return fail ("back: L should equal R by symmetry");

    // ── Sweep around the head, dump and check: ipsilateral ear should
    //    dominate in the canonical lateral hemisphere (45°..135°). The
    //    order-3 sampling decoder has side lobes near the antipode that
    //    can flip dominance — that's a known limitation of the prototype
    //    (real MagLS in step 6+ fixes it via a least-squares fit).
    std::cout << "sweep:\n";
    for (float deg = -180.0f; deg <= 180.0f; deg += 15.0f)
    {
        const auto az = deg * kPi / 180.0f;
        const auto s  = encodeAndDecode (enc, dec, az, 0.0f, 1.0f);
        std::cout << "  az=" << deg << "°  L=" << s.l << "  R=" << s.r << '\n';
        if (deg >= 45.0f && deg <= 135.0f && s.l <= s.r)
            return fail ("left lateral hemisphere: L should dominate");
        if (deg >= -135.0f && deg <= -45.0f && s.r <= s.l)
            return fail ("right lateral hemisphere: R should dominate");
    }

    // ── Up-axis: source overhead should leave L = R (azimuth-independent). ──
    const auto up = encodeAndDecode (enc, dec, 0.0f, kHalfPi, 1.0f);
    std::cout << "source @ up       : L=" << up.l       << "  R=" << up.r       << '\n';
    if (std::abs (up.l - up.r) > 1e-4f)
        return fail ("up: L should equal R");

    std::cout << "PASS\n";
    return 0;
}
