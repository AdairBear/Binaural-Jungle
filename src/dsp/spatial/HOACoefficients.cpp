#include "HOACoefficients.h"

#include <cmath>

namespace bjf::spatial
{

void computeAcnSn3d (float az, float el, float* y) noexcept
{
    const float ce  = std::cos (el);
    const float se  = std::sin (el);
    const float ce2 = ce * ce;
    const float se2 = se * se;

    const float ca  = std::cos (az);
    const float sa  = std::sin (az);
    const float c2a = std::cos (2.0f * az);
    const float s2a = std::sin (2.0f * az);
    const float c3a = std::cos (3.0f * az);
    const float s3a = std::sin (3.0f * az);

    // ── Order 0 ──
    y[0] = 1.0f;                                    // (0, 0)

    // ── Order 1, SN3D ──
    y[1] = sa * ce;                                 // (1,-1) — y axis (left)
    y[2] = se;                                      // (1, 0) — z axis (up)
    y[3] = ca * ce;                                 // (1, 1) — x axis (front)

    // ── Order 2, SN3D ──
    constexpr float k22 = 0.8660254037844386f;      // sqrt(3) / 2
    y[4] = k22 * s2a * ce2;                         // (2,-2)
    y[5] = k22 * sa  * 2.0f * se * ce;              // (2,-1)
    y[6] = 0.5f * (3.0f * se2 - 1.0f);              // (2, 0)
    y[7] = k22 * ca  * 2.0f * se * ce;              // (2, 1)
    y[8] = k22 * c2a * ce2;                         // (2, 2)

    // ── Order 3, SN3D ──
    constexpr float k33 = 0.7905694150420948f;      // sqrt(5/8)
    constexpr float k32 = 1.9364916731037085f;      // sqrt(15) / 2
    constexpr float k31 = 0.6123724356957945f;      // sqrt(3/8)
    y[9]  = k33 * s3a * ce2 * ce;                   // (3,-3)
    y[10] = k32 * s2a * se  * ce2;                  // (3,-2)
    y[11] = k31 * sa  * ce  * (5.0f * se2 - 1.0f);  // (3,-1)
    y[12] = 0.5f * se  * (5.0f * se2 - 3.0f);       // (3, 0)
    y[13] = k31 * ca  * ce  * (5.0f * se2 - 1.0f);  // (3, 1)
    y[14] = k32 * c2a * se  * ce2;                  // (3, 2)
    y[15] = k33 * c3a * ce2 * ce;                   // (3, 3)
}

} // namespace bjf::spatial
