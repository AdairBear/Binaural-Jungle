#pragma once

#include <string>
#include <vector>

// Forward declaration so consumers don't need to drag in libmysofa headers.
struct MYSOFA_HRTF;

namespace bjf::spatial
{

// Wraps libmysofa's HRTF reader. After a successful load the loader owns a
// flat in-memory representation of the SOFA file's measurements:
//   - sourcePositions: M directions, each (azimuth_rad, elevation_rad, radius_m)
//   - hrirs:           M·R·N floats, contiguous, row-major [direction][ear][tap]
// where M = number of measurements, R = receivers (always 2 for binaural),
// N = filter length per measurement.
//
// The libmysofa-side MYSOFA_HRTF is freed before loadFromFile returns — we
// own only the converted flat arrays. This keeps the rest of the spatial
// module free of libmysofa types and makes the loader trivial to test in
// isolation.
class SOFALoader
{
public:
    struct LoadStatus
    {
        bool        success      = false;
        int         libmysofaErr = 0;
        std::string message;
    };

    LoadStatus loadFromFile (const std::string& path);

    bool   isLoaded() const noexcept             { return loaded; }
    int    getNumDirections() const noexcept     { return numDirections; }
    int    getFilterLength() const noexcept      { return filterLength; }
    float  getSampleRate() const noexcept        { return sampleRate; }
    int    getNumReceivers() const noexcept      { return numReceivers; }

    // Returns the (azRad, elRad) for measurement m. Out of range → (0, 0).
    void getDirection (int m, float& azRad, float& elRad) const noexcept;

    // Returns a pointer to N consecutive HRIR taps for measurement m, ear e.
    // e == 0 → left ear, e == 1 → right ear. Returns nullptr if not loaded
    // or m / e out of range.
    const float* getHRIR (int m, int ear) const noexcept;

private:
    bool  loaded         = false;
    int   numDirections  = 0;
    int   filterLength   = 0;
    int   numReceivers   = 0;
    float sampleRate     = 0.0f;

    // M × 2 floats: [m*2 + 0] = az_rad, [m*2 + 1] = el_rad. Radius is
    // ignored — we treat all measurements as far-field for the prototype.
    std::vector<float> directions;

    // M·R·N floats, row-major over [m][r][n].
    std::vector<float> hrirs;
};

} // namespace bjf::spatial
