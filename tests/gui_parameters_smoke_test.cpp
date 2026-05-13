// Compile-time + tiny runtime sanity check that every parameter ID the new
// full-GUI binds to is present in ParameterIDs.h. The test is intentionally
// hermetic — it does not instantiate the audio processor, so it can compile
// and run without pulling in JUCE plugin infrastructure or the DSP graph.
//
// The real consistency check between ParameterIDs.h and the
// createParameterLayout() call sites lives in the production build itself
// (every ID is constexpr; a typo in either site fails to compile). This test
// catches the orthogonal failure: a new GUI panel referencing a parameter
// that was forgotten in ParameterIDs.h, by enumerating expected IDs by hand.

#include "../src/app/ParameterIDs.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace
{
int fail (const char* msg)
{
    std::cerr << "FAIL: " << msg << '\n';
    return 1;
}

bool contains (const char* needle)
{
    for (int i = 0; i < bjf::pid::kCount; ++i)
        if (std::strcmp (bjf::pid::kAll[i], needle) == 0)
            return true;
    return false;
}
} // namespace

int main()
{
    // Every parameter ID the new GUI panels touch — keep this list in sync
    // with src/gui/Panels.cpp and the rest of the editor wiring.
    const std::vector<const char*> required = {
        // Oscillator
        bjf::pid::waveform, bjf::pid::oscOctave, bjf::pid::detuneCents,
        bjf::pid::oscPhase, bjf::pid::stackSize, bjf::pid::oscSpread,
        // Granular
        bjf::pid::granSample, bjf::pid::granDensity, bjf::pid::granSizeMs,
        bjf::pid::granPitch, bjf::pid::granScatter, bjf::pid::granSpray,
        bjf::pid::granMix,
        // Filter
        bjf::pid::filterType, bjf::pid::filterCutoff, bjf::pid::filterReso,
        bjf::pid::filterEnvAmount, bjf::pid::filterKeyTrack, bjf::pid::filterDrive,
        // Envelope
        bjf::pid::envDelay, bjf::pid::attack, bjf::pid::envHold, bjf::pid::decay,
        bjf::pid::sustain, bjf::pid::release, bjf::pid::envCurve,
        // Spatial
        bjf::pid::spatialAz, bjf::pid::spatialEl,
        bjf::pid::spatialSpreadAz, bjf::pid::spatialSpreadEl,
        // ER + Reverb
        bjf::pid::erRoomSize, bjf::pid::erWallDamping, bjf::pid::erMix,
        bjf::pid::revRoomSize, bjf::pid::revDecay, bjf::pid::revDamping,
        bjf::pid::revPreDelay, bjf::pid::revDiffusion, bjf::pid::revWetDry,
        // Master
        bjf::pid::gain, bjf::pid::pan, bjf::pid::voices, bjf::pid::bpm
    };

    for (const auto* id : required)
    {
        if (id == nullptr || *id == '\0')
            return fail ("ID is empty");
        if (! contains (id))
        {
            std::cerr << "  missing id: " << id << '\n';
            return fail ("required parameter ID not in pid::kAll");
        }
    }

    // No duplicate IDs allowed — APVTS would refuse to register them, but
    // catching it here gives a clearer error message than the JUCE assert.
    std::unordered_set<std::string> seen;
    for (int i = 0; i < bjf::pid::kCount; ++i)
    {
        std::string s { bjf::pid::kAll[i] };
        if (! seen.insert (s).second)
        {
            std::cerr << "  duplicate id: " << s << '\n';
            return fail ("duplicate parameter ID in pid::kAll");
        }
    }

    if (bjf::pid::kCount < static_cast<int> (required.size()))
        return fail ("pid::kCount is smaller than the required set — header out of sync");

    std::cout << "PASS — "
              << required.size() << " required ids, "
              << bjf::pid::kCount << " total registered\n";
    return 0;
}
