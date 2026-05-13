#include "EarlyReflections.h"

#include <algorithm>
#include <cmath>

namespace bjf::spatial
{

namespace
{
    // Six wall directions, in (sign-x, sign-y, sign-z) form. Walls at the
    // half-widths along each axis, so the image of a source across wall
    // (sx*halfX, *, *) is at x = 2 * sx * halfX − src_x.
    //
    // Order convention: the table indices map 1:1 to test expectations, so
    // do not reorder once tests depend on them.
    struct WallAxis { int axis; float sign; };
    constexpr std::array<WallAxis, 6> kWalls = {{
        { 0, +1.0f },  // +X (front)
        { 0, -1.0f },  // -X (back)
        { 1, +1.0f },  // +Y (left)
        { 1, -1.0f },  // -Y (right)
        { 2, +1.0f },  // +Z (ceiling)
        { 2, -1.0f },  // -Z (floor)
    }};
}

EarlyReflections::EarlyReflections()
{
    delayBuffer.assign (kDelayBufferSize, 0.0f);
}

void EarlyReflections::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 48000.0;
    reset();
    recomputeReflections();
}

void EarlyReflections::reset() noexcept
{
    std::fill (delayBuffer.begin(), delayBuffer.end(), 0.0f);
    writeIndex = 0;
    for (auto& r : reflections)
        r.lpfState = 0.0f;
}

void EarlyReflections::setRoomSize (float meters) noexcept
{
    const auto clamped = std::clamp (meters, 1.0f, 30.0f);
    if (clamped == roomSize) return;
    roomSize = clamped;
    recomputeReflections();
}

void EarlyReflections::setWallDamping (float damping01) noexcept
{
    wallDamping = std::clamp (damping01, 0.0f, 1.0f);
    recomputeReflections();
}

void EarlyReflections::setMix (float mix01) noexcept
{
    mix = std::clamp (mix01, 0.0f, 1.0f);
}

void EarlyReflections::recomputeReflections() noexcept
{
    // Half-widths of the shoebox along each axis. Listener at origin; walls
    // at ±halfDim per axis.
    const float halfDim[3] = {
        0.5f * roomSize * kAspectX,
        0.5f * roomSize * kAspectY,
        0.5f * roomSize * kAspectZ,
    };

    const float srcPos[3] = { kSourceX, kSourceY, kSourceZ };

    // Map [0..1] damping to LPF feedback coefficient α in [0.0..0.85]. At
    // α=0 the LPF is a passthrough (no damping); at α=0.85 the −3 dB point
    // sits around 1.2 kHz at 48 kHz — heavily damped without ringing.
    const float lpfMax = 0.85f;
    const float lpfA   = wallDamping * lpfMax;

    for (std::size_t i = 0; i < reflections.size(); ++i)
    {
        const auto& wall = kWalls[i];

        // Image source: mirror across the wall in the wall's axis, leave the
        // other two axes unchanged. Source mirrored across plane x = sign*halfX:
        //   image_axis = 2 * sign * halfDim[axis] − src[axis]
        float imgPos[3] = { srcPos[0], srcPos[1], srcPos[2] };
        imgPos[wall.axis] = 2.0f * wall.sign * halfDim[wall.axis]
                          - srcPos[wall.axis];

        // Distance from listener (origin) to image, with a small floor to
        // keep the gain finite if a wall is configured pathologically close.
        const float distance = std::max (0.05f,
            std::sqrt (imgPos[0] * imgPos[0]
                     + imgPos[1] * imgPos[1]
                     + imgPos[2] * imgPos[2]));

        // Spherical direction (azimuth, elevation) in the project's
        // convention: az = 0 → +x (front), az = +π/2 → +y (left),
        // el = +π/2 → +z (up). std::atan2 returns (-π, π] which matches.
        const float az = std::atan2 (imgPos[1], imgPos[0]);
        const float el = std::asin  (std::clamp (imgPos[2] / distance, -1.0f, 1.0f));

        auto& r = reflections[i];

        // Delay in samples, clamped to fit inside the ring buffer with at
        // least one sample of headroom so the read can never collide with
        // the write position.
        const int delay = static_cast<int> (std::round (
            distance * static_cast<float> (sampleRate) / kSpeedOfSound));
        r.delaySamples = std::clamp (delay, 1, kDelayBufferSize - 1);

        // 1 / r geometric falloff, capped at unity so a tiny room doesn't
        // produce reflections louder than the dry signal.
        r.gain = std::min (1.0f, 1.0f / distance);

        r.lpfA = lpfA;
        r.lpfState = 0.0f;

        computeAcnSn3d (az, el, r.hoaCoefs.data());
    }
}

void EarlyReflections::processSample (const float* hoaIn, float* hoaOut) noexcept
{
    // Mono drive for the model: W channel is 1·mono under our SN3D encode
    // (y[0] = 1), so hoaIn[0] is the scene-as-mono signal directly.
    const float mono = hoaIn[0];

    delayBuffer[(std::size_t) writeIndex] = mono;

    for (auto& r : reflections)
    {
        const int readIdx = (writeIndex - r.delaySamples) & kDelayMask;
        const float tap   = delayBuffer[(std::size_t) readIdx];

        // One-pole low-pass: y[n] = (1 − α)·x[n] + α·y[n−1]. α = 0 → bypass.
        r.lpfState = (1.0f - r.lpfA) * tap + r.lpfA * r.lpfState;

        const float reflectionSample = r.lpfState * r.gain * mix;

        // Encode the (already-attenuated) sample into HOA at the image
        // direction. Coefficients are pre-computed in recomputeReflections().
        for (int c = 0; c < kNumHoaChannels; ++c)
            hoaOut[c] += reflectionSample * r.hoaCoefs[(std::size_t) c];
    }

    writeIndex = (writeIndex + 1) & kDelayMask;
}

} // namespace bjf::spatial
