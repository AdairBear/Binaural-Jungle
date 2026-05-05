// Smoke test for the order-3 ACN/SN3D real spherical-harmonic coefficients.
// Verifies the closed-form values at canonical directions (front, left, up,
// right, back) match expectation, and that the full coefficient vector is
// finite at a sweep of azimuths.

#include "../src/dsp/spatial/HOACoefficients.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr float kPi     = 3.141592653589793f;
constexpr float kHalfPi = 1.5707963267948966f;
constexpr float kEps    = 1e-5f;

bool approx (float a, float b) noexcept { return std::abs (a - b) < kEps; }

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }

void dump (const char* label, const float* y)
{
    std::cout << label << ": ";
    for (int i = 0; i < bjf::spatial::kNumHoaChannels; ++i)
        std::cout << y[i] << (i + 1 < bjf::spatial::kNumHoaChannels ? ' ' : '\n');
}
}

int main()
{
    using namespace bjf::spatial;
    float y[kNumHoaChannels];

    // ── Front (az=0, el=0): Y(0,0)=1, Y(1,1)=1 (x axis), Y(1,-1)=0, Y(1,0)=0 ──
    computeAcnSn3d (0.0f, 0.0f, y);
    dump ("front  ", y);
    if (! approx (y[0], 1.0f))   return fail ("front: Y(0,0) != 1");
    if (! approx (y[1], 0.0f))   return fail ("front: Y(1,-1) != 0");
    if (! approx (y[2], 0.0f))   return fail ("front: Y(1, 0) != 0");
    if (! approx (y[3], 1.0f))   return fail ("front: Y(1, 1) != 1");

    // ── Left (az=+π/2, el=0): Y(1,-1)=1, Y(1,1)=0 ──
    computeAcnSn3d (+kHalfPi, 0.0f, y);
    dump ("left   ", y);
    if (! approx (y[1], 1.0f))   return fail ("left: Y(1,-1) != 1");
    if (! approx (y[3], 0.0f))   return fail ("left: Y(1, 1) != 0");

    // ── Right (az=-π/2, el=0): Y(1,-1)=-1, Y(1,1)=0 ──
    computeAcnSn3d (-kHalfPi, 0.0f, y);
    dump ("right  ", y);
    if (! approx (y[1], -1.0f))  return fail ("right: Y(1,-1) != -1");
    if (! approx (y[3], 0.0f))   return fail ("right: Y(1, 1) != 0");

    // ── Up (az=0, el=+π/2): Y(1,0)=1, Y(1,1)=0, Y(1,-1)=0 ──
    computeAcnSn3d (0.0f, kHalfPi, y);
    dump ("up     ", y);
    if (! approx (y[1], 0.0f))   return fail ("up: Y(1,-1) != 0");
    if (! approx (y[2], 1.0f))   return fail ("up: Y(1, 0) != 1");
    if (! approx (y[3], 0.0f))   return fail ("up: Y(1, 1) != 0");

    // ── Back (az=π, el=0): Y(1,1)=-1 ──
    computeAcnSn3d (kPi, 0.0f, y);
    dump ("back   ", y);
    if (! approx (y[3], -1.0f))  return fail ("back: Y(1, 1) != -1");

    // ── No NaNs / no Infs across an azimuth sweep ──
    for (int i = 0; i < 360; ++i)
    {
        const auto az = (static_cast<float> (i) - 180.0f) * kPi / 180.0f;
        for (float el : { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f })
        {
            computeAcnSn3d (az, el, y);
            for (int n = 0; n < kNumHoaChannels; ++n)
                if (! std::isfinite (y[n]))
                    return fail ("non-finite SH coefficient encountered in sweep");
        }
    }

    std::cout << "PASS\n";
    return 0;
}
