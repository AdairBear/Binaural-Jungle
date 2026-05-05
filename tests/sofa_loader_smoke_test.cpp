// Smoke test for bjf::spatial::SOFALoader. Loads the MIT KEMAR file shipped
// with libmysofa and verifies the metadata + HRIR data look sensible.

#include "../src/dsp/spatial/SOFALoader.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

#ifndef BJF_DEFAULT_SOFA_PATH
 #error "BJF_DEFAULT_SOFA_PATH not defined — CMake should provide it"
#endif

namespace
{
int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }
}

int main()
{
    bjf::spatial::SOFALoader loader;

    const auto status = loader.loadFromFile (BJF_DEFAULT_SOFA_PATH);
    if (! status.success)
    {
        std::cerr << "load failed: " << status.message
                  << " (libmysofa err " << status.libmysofaErr << ")\n";
        return fail ("could not load default SOFA file");
    }

    const int M = loader.getNumDirections();
    const int N = loader.getFilterLength();
    const int R = loader.getNumReceivers();
    const float fs = loader.getSampleRate();

    std::cout << "SOFA: M=" << M << "  N=" << N << "  R=" << R
              << "  fs=" << fs << '\n';

    if (M < 16)        return fail ("too few measurement directions for order-3");
    if (N < 32)        return fail ("HRIR filter length suspiciously small");
    if (R != 2)        return fail ("expected 2 receivers (binaural)");
    if (fs < 22000.0f) return fail ("sample rate suspiciously low");

    // Spot-check a few HRIRs: each should be non-zero and finite.
    for (int m : { 0, M / 4, M / 2, 3 * M / 4, M - 1 })
    {
        for (int ear : { 0, 1 })
        {
            const float* ir = loader.getHRIR (m, ear);
            if (ir == nullptr) return fail ("getHRIR returned null on valid input");
            float peak = 0.0f;
            for (int n = 0; n < N; ++n)
            {
                if (! std::isfinite (ir[n])) return fail ("HRIR contains non-finite tap");
                peak = std::max (peak, std::abs (ir[n]));
            }
            if (peak < 1e-5f) return fail ("HRIR peak suspiciously close to zero");
        }
    }

    // Direction sweep: expect azimuths spread roughly across 2π.
    float minAz = +1e9f, maxAz = -1e9f;
    for (int m = 0; m < M; ++m)
    {
        float az = 0.0f, el = 0.0f;
        loader.getDirection (m, az, el);
        minAz = std::min (minAz, az);
        maxAz = std::max (maxAz, az);
    }
    std::cout << "azimuth range (rad): [" << minAz << ", " << maxAz << "]\n";
    if (maxAz - minAz < 1.0f) return fail ("azimuth range too narrow — bad SOFA?");

    std::cout << "PASS\n";
    return 0;
}
