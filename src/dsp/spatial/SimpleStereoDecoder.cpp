#include "SimpleStereoDecoder.h"

#include <cmath>

namespace bjf::spatial
{

namespace
{
constexpr float kHalfPi = 1.5707963267948966f;
}

SimpleStereoDecoder::SimpleStereoDecoder() noexcept
{
    // Left ear at azimuth +π/2, elevation 0. Right ear at -π/2, 0.
    computeAcnSn3d (+kHalfPi, 0.0f, leftCoefs);
    computeAcnSn3d (-kHalfPi, 0.0f, rightCoefs);

    // Self-energy of either column at order 3 is exactly 4. Verify and use
    // the analytic value so we don't pay sqrt + division at construction.
    float energy = 0.0f;
    for (int i = 0; i < kNumHoaChannels; ++i)
        energy += leftCoefs[i] * leftCoefs[i];

    // Guard against a future order change breaking the analytic constant.
    scale = (energy > 1.0e-6f) ? (1.0f / energy) : 0.25f;
}

void SimpleStereoDecoder::decodeSample (const float* hoa16,
                                        float& outL, float& outR) const noexcept
{
    float l = 0.0f;
    float r = 0.0f;
    for (int i = 0; i < kNumHoaChannels; ++i)
    {
        l += leftCoefs[i]  * hoa16[i];
        r += rightCoefs[i] * hoa16[i];
    }
    outL = scale * l;
    outR = scale * r;
}

} // namespace bjf::spatial
