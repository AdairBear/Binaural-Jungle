#include "HOAEncoder.h"

namespace bjf::spatial
{

HOAEncoder::HOAEncoder() noexcept
{
    setPosition (0.0f, 0.0f);
}

void HOAEncoder::setPosition (float az, float el) noexcept
{
    // 16 SH evaluations are cheap enough to redo unconditionally per call;
    // dodging a float == comparison keeps -Wfloat-equal clean.
    computeAcnSn3d (az, el, coefs);
}

void HOAEncoder::encodeSample (float monoIn, float* out) const noexcept
{
    for (int i = 0; i < kNumHoaChannels; ++i)
        out[i] = monoIn * coefs[i];
}

} // namespace bjf::spatial
