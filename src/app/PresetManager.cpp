#include "PresetManager.h"

#include "ParameterIDs.h"

#include <utility>

namespace bjf
{

namespace
{
    // ── Factory preset table ───────────────────────────────────────────────
    // Each preset is a list of (parameter id, raw value) overrides. Anything
    // not listed stays at its createParameterLayout default — that's what
    // makes "Init" trivially a no-override entry.
    //
    // Values are in display units (Hz, seconds, degrees, dB, ...). The
    // PresetManager normalises them on apply via convertTo0to1().
    //
    // Design notes — the 20 presets below trace the canonical jungle-pad
    // arc from Logical Progression (LTJ Bukem) through Timeless (Goldie),
    // Two Pages (4hero) and Modus Operandi (Photek), each cut emphasising
    // a different corner of the synth: huge oscillator stacks for the
    // strings; granular for the Photek-style abstractions; long FDN
    // decays for the Bukem washes.
    struct ParamOverride { const char* id; float value; };

    struct FactoryDef
    {
        const char* name;
        std::vector<ParamOverride> overrides;
    };

    const std::vector<FactoryDef>& factoryTable()
    {
        // First-time entry constructs the table; subsequent calls return
        // the same instance. Safe at static-init time because pid::* are
        // constexpr.
        static const std::vector<FactoryDef> table = {
            // 0 — Init: every parameter at its default. Useful sound-design
            // starting point and the value the user gets from the [Init]
            // button on the preset bar.
            { "Init", { } },

            // 1 — Logical Pad
            // LTJ Bukem 'Logical Progression' signature. Massive sawtooth
            // stack, gentle high-cut, long attack and tail, wide spread.
            { "Logical Pad", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      6.0f },
                { pid::detuneCents,    14.0f },
                { pid::oscSpread,      0.7f },
                { pid::filterType,     0.0f },
                { pid::filterCutoff,   1900.0f },
                { pid::filterReso,     1.0f },
                { pid::filterKeyTrack, 0.3f },
                { pid::attack,         1.2f },
                { pid::decay,          1.5f },
                { pid::sustain,        0.85f },
                { pid::release,        3.5f },
                { pid::spatialSpreadAz, 220.0f },
                { pid::spatialSpreadEl, 30.0f },
                { pid::erRoomSize,     14.0f },
                { pid::erMix,          0.25f },
                { pid::revDecay,       5.0f },
                { pid::revDamping,     0.45f },
                { pid::revPreDelay,    35.0f },
                { pid::revWetDry,      0.42f },
                { pid::gain,           -7.0f },
            }},

            // 2 — Horizons
            // Bukem-era upper-register sparkle pad. Brighter cutoff, smaller
            // stack but wider detune, shorter tail than Logical Pad.
            { "Horizons", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      5.0f },
                { pid::detuneCents,    22.0f },
                { pid::oscSpread,      0.55f },
                { pid::filterCutoff,   4200.0f },
                { pid::filterReso,     0.9f },
                { pid::attack,         0.8f },
                { pid::release,        2.6f },
                { pid::spatialSpreadAz, 180.0f },
                { pid::spatialSpreadEl, 18.0f },
                { pid::revDecay,       3.6f },
                { pid::revWetDry,      0.38f },
                { pid::gain,           -8.0f },
            }},

            // 3 — Music
            // Drifting ambient bed. Very slow envelope, low filter so the
            // pad sits underneath everything; reverb does most of the work.
            { "Music", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      7.0f },
                { pid::detuneCents,    9.0f },
                { pid::filterCutoff,   900.0f },
                { pid::filterReso,     0.8f },
                { pid::attack,         2.4f },
                { pid::decay,          2.0f },
                { pid::sustain,        0.95f },
                { pid::release,        6.0f },
                { pid::spatialSpreadAz, 280.0f },
                { pid::spatialSpreadEl, 45.0f },
                { pid::erRoomSize,     18.0f },
                { pid::erMix,          0.3f },
                { pid::revDecay,       8.0f },
                { pid::revDamping,     0.55f },
                { pid::revWetDry,      0.55f },
                { pid::gain,           -9.0f },
            }},

            // 4 — Atlantis
            // Sub-low pad. Octave down, fewer voices needed, narrow spread
            // so the bottom stays mono-summed at the speakers.
            { "Atlantis", {
                { pid::waveform,       0.0f },
                { pid::oscOctave,      -1.0f },
                { pid::stackSize,      3.0f },
                { pid::detuneCents,    5.0f },
                { pid::filterCutoff,   600.0f },
                { pid::filterReso,     1.2f },
                { pid::attack,         0.6f },
                { pid::release,        3.0f },
                { pid::spatialSpreadAz, 60.0f },
                { pid::erMix,          0.15f },
                { pid::revDecay,       4.0f },
                { pid::revWetDry,      0.3f },
                { pid::gain,           -6.0f },
            }},

            // 5 — Timeless Strings
            // Goldie 'Timeless' wide ensemble strings. Big stack, square
            // tail for orchestral bite, expansive cinematic spread.
            { "Timeless Strings", {
                { pid::waveform,       1.0f },
                { pid::stackSize,      7.0f },
                { pid::detuneCents,    18.0f },
                { pid::oscSpread,      0.8f },
                { pid::filterCutoff,   3200.0f },
                { pid::filterReso,     1.1f },
                { pid::attack,         0.9f },
                { pid::decay,          0.8f },
                { pid::sustain,        0.9f },
                { pid::release,        4.5f },
                { pid::spatialSpreadAz, 300.0f },
                { pid::spatialSpreadEl, 60.0f },
                { pid::erRoomSize,     22.0f },
                { pid::erMix,          0.32f },
                { pid::revDecay,       6.5f },
                { pid::revPreDelay,    45.0f },
                { pid::revWetDry,      0.45f },
                { pid::gain,           -7.0f },
            }},

            // 6 — Inner City
            // Warm narrow intro pad. Sits mid-stage, modest spread so
            // melody-carrying chords stay focused.
            { "Inner City", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      4.0f },
                { pid::detuneCents,    11.0f },
                { pid::filterCutoff,   1500.0f },
                { pid::filterReso,     0.95f },
                { pid::attack,         0.4f },
                { pid::release,        2.2f },
                { pid::spatialSpreadAz, 140.0f },
                { pid::revDecay,       3.0f },
                { pid::revWetDry,      0.32f },
                { pid::gain,           -7.0f },
            }},

            // 7 — Mothership Connection
            // Cinematic widescreen pad — maximum spread, long reverb,
            // moderate filter to keep weight at the bottom.
            { "Mothership Connection", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      7.0f },
                { pid::detuneCents,    30.0f },
                { pid::oscSpread,      0.9f },
                { pid::filterCutoff,   2400.0f },
                { pid::attack,         1.6f },
                { pid::release,        5.0f },
                { pid::sustain,        0.9f },
                { pid::spatialSpreadAz, 340.0f },
                { pid::spatialSpreadEl, 80.0f },
                { pid::erRoomSize,     26.0f },
                { pid::erWallDamping,  0.25f },
                { pid::erMix,          0.4f },
                { pid::revDecay,       7.0f },
                { pid::revPreDelay,    55.0f },
                { pid::revWetDry,      0.5f },
                { pid::gain,           -9.0f },
            }},

            // 8 — Two Pages
            // 4hero jazz chord pad. Brighter saw, key-tracking filter for
            // open feel, short reverb to keep transients audible.
            { "Two Pages", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      4.0f },
                { pid::detuneCents,    8.0f },
                { pid::filterCutoff,   5200.0f },
                { pid::filterReso,     0.85f },
                { pid::filterKeyTrack, 0.6f },
                { pid::attack,         0.25f },
                { pid::decay,          0.7f },
                { pid::sustain,        0.8f },
                { pid::release,        1.8f },
                { pid::spatialSpreadAz, 160.0f },
                { pid::revDecay,       2.4f },
                { pid::revDamping,     0.35f },
                { pid::revWetDry,      0.28f },
                { pid::gain,           -6.0f },
            }},

            // 9 — Crystal Vision
            // 4hero shimmering high pad. Open filter, fast attack, narrow
            // detune so the high partials don't beat audibly.
            { "Crystal Vision", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      5.0f },
                { pid::detuneCents,    6.0f },
                { pid::oscSpread,      0.4f },
                { pid::filterCutoff,   8500.0f },
                { pid::filterReso,     0.9f },
                { pid::attack,         0.35f },
                { pid::release,        2.0f },
                { pid::spatialSpreadAz, 200.0f },
                { pid::spatialSpreadEl, 40.0f },
                { pid::revDecay,       3.2f },
                { pid::revWetDry,      0.35f },
                { pid::gain,           -8.0f },
            }},

            // 10 — Star Chasers
            // High glittery pad. Octave up, smaller stack, bright filter.
            { "Star Chasers", {
                { pid::waveform,       0.0f },
                { pid::oscOctave,      1.0f },
                { pid::stackSize,      4.0f },
                { pid::detuneCents,    15.0f },
                { pid::filterCutoff,   6800.0f },
                { pid::filterReso,     1.3f },
                { pid::attack,         0.5f },
                { pid::release,        2.4f },
                { pid::spatialSpreadAz, 250.0f },
                { pid::spatialSpreadEl, 55.0f },
                { pid::revDecay,       4.2f },
                { pid::revWetDry,      0.38f },
                { pid::gain,           -9.0f },
            }},

            // 11 — Modus Operandi
            // Photek abstract texture — granular doing the heavy lifting,
            // oscillator stack is a quiet subharmonic bed underneath.
            { "Modus Operandi", {
                { pid::waveform,       1.0f },
                { pid::stackSize,      2.0f },
                { pid::detuneCents,    4.0f },
                { pid::granSample,     1.0f },   // Slot 1
                { pid::granDensity,    0.55f },
                { pid::granSizeMs,     90.0f },
                { pid::granScatter,    0.5f },
                { pid::granSpray,      0.35f },
                { pid::granMix,        0.7f },
                { pid::filterCutoff,   1100.0f },
                { pid::filterReso,     2.2f },
                { pid::filterDrive,    0.35f },
                { pid::attack,         0.05f },
                { pid::release,        2.5f },
                { pid::spatialSpreadAz, 240.0f },
                { pid::erRoomSize,     12.0f },
                { pid::erMix,          0.35f },
                { pid::revDecay,       3.8f },
                { pid::revDamping,     0.7f },
                { pid::revWetDry,      0.38f },
                { pid::gain,           -7.0f },
            }},

            // 12 — The Hidden Camera
            // Dark Photek pad. Low cutoff, resonance peak, slow stalking
            // attack-release pattern.
            { "The Hidden Camera", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      4.0f },
                { pid::detuneCents,    7.0f },
                { pid::filterCutoff,   480.0f },
                { pid::filterReso,     3.5f },
                { pid::filterDrive,    0.45f },
                { pid::attack,         1.4f },
                { pid::release,        3.2f },
                { pid::spatialSpreadAz, 120.0f },
                { pid::revDecay,       4.0f },
                { pid::revDamping,     0.75f },
                { pid::revWetDry,      0.4f },
                { pid::gain,           -7.0f },
            }},

            // 13 — Ni Ten Ichi Ryu
            // Photek granular cinematic. Two grain sources implied via
            // higher density + spray. Long pre-delay puts taps behind it.
            { "Ni Ten Ichi Ryu", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      3.0f },
                { pid::granSample,     2.0f },
                { pid::granDensity,    0.7f },
                { pid::granSizeMs,     140.0f },
                { pid::granPitch,      -7.0f },
                { pid::granScatter,    0.6f },
                { pid::granSpray,      0.4f },
                { pid::granMix,        0.6f },
                { pid::filterCutoff,   2200.0f },
                { pid::attack,         0.4f },
                { pid::release,        4.0f },
                { pid::spatialSpreadAz, 260.0f },
                { pid::spatialSpreadEl, 50.0f },
                { pid::erRoomSize,     18.0f },
                { pid::erMix,          0.3f },
                { pid::revDecay,       5.5f },
                { pid::revPreDelay,    80.0f },
                { pid::revWetDry,      0.45f },
                { pid::gain,           -8.0f },
            }},

            // 14 — Demons Theme
            // Goldie sinister stab-into-pad. Slightly aggressive square,
            // mid-cut filter, mean drive.
            { "Demons Theme", {
                { pid::waveform,       1.0f },
                { pid::stackSize,      5.0f },
                { pid::detuneCents,    16.0f },
                { pid::oscOctave,      -1.0f },
                { pid::filterCutoff,   1700.0f },
                { pid::filterReso,     2.0f },
                { pid::filterDrive,    0.6f },
                { pid::attack,         0.15f },
                { pid::release,        2.8f },
                { pid::spatialSpreadAz, 180.0f },
                { pid::revDecay,       3.5f },
                { pid::revWetDry,      0.35f },
                { pid::gain,           -6.0f },
            }},

            // 15 — Spirit of the Sun
            // Bukem golden warmth. Slight square edge, medium stack, very
            // wide envelope, gold-rich mids around 2 kHz.
            { "Spirit of the Sun", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      6.0f },
                { pid::detuneCents,    12.0f },
                { pid::filterCutoff,   2800.0f },
                { pid::filterReso,     1.0f },
                { pid::filterKeyTrack, 0.25f },
                { pid::attack,         1.0f },
                { pid::release,        4.0f },
                { pid::spatialSpreadAz, 210.0f },
                { pid::erMix,          0.22f },
                { pid::revDecay,       4.8f },
                { pid::revWetDry,      0.4f },
                { pid::gain,           -7.0f },
            }},

            // 16 — Sky's the Limit
            // Open airy pad. Octave up, generous spread, long shimmery
            // reverb. Sits clearly above any drum bus.
            { "Sky's the Limit", {
                { pid::waveform,       0.0f },
                { pid::oscOctave,      1.0f },
                { pid::stackSize,      4.0f },
                { pid::detuneCents,    10.0f },
                { pid::filterCutoff,   7200.0f },
                { pid::filterReso,     0.9f },
                { pid::attack,         0.7f },
                { pid::release,        3.0f },
                { pid::spatialSpreadAz, 320.0f },
                { pid::spatialSpreadEl, 70.0f },
                { pid::revDecay,       6.0f },
                { pid::revDamping,     0.3f },
                { pid::revWetDry,      0.48f },
                { pid::gain,           -10.0f },
            }},

            // 17 — Subterranean
            // Deep sub pad. Two octaves down, mono-ish placement, dry — for
            // anchoring a mix where the spatial work happens above it.
            { "Subterranean", {
                { pid::waveform,       0.0f },
                { pid::oscOctave,      -2.0f },
                { pid::stackSize,      2.0f },
                { pid::detuneCents,    3.0f },
                { pid::filterCutoff,   450.0f },
                { pid::filterReso,     0.95f },
                { pid::attack,         0.05f },
                { pid::release,        1.6f },
                { pid::spatialSpreadAz, 30.0f },
                { pid::erMix,          0.0f },
                { pid::revWetDry,      0.15f },
                { pid::gain,           -5.0f },
            }},

            // 18 — Stalker
            // Movement-driven dark pad. Granular spray adds texture but
            // doesn't dominate.
            { "Stalker", {
                { pid::waveform,       1.0f },
                { pid::stackSize,      4.0f },
                { pid::detuneCents,    19.0f },
                { pid::granSample,     3.0f },
                { pid::granDensity,    0.3f },
                { pid::granSizeMs,     180.0f },
                { pid::granPitch,      -12.0f },
                { pid::granScatter,    0.45f },
                { pid::granSpray,      0.5f },
                { pid::granMix,        0.35f },
                { pid::filterType,     1.0f },
                { pid::filterCutoff,   1300.0f },
                { pid::filterReso,     2.6f },
                { pid::filterDrive,    0.4f },
                { pid::attack,         0.9f },
                { pid::release,        3.4f },
                { pid::spatialSpreadAz, 230.0f },
                { pid::revDecay,       4.6f },
                { pid::revDamping,     0.65f },
                { pid::revWetDry,      0.38f },
                { pid::gain,           -7.0f },
            }},

            // 19 — Sketches
            // Abstract granular sketchpad. Oscillator stack is silent
            // backing; grains carry the entire timbre. Spray and scatter
            // maxed out for full cloud-of-grains behaviour.
            { "Sketches", {
                { pid::waveform,       0.0f },
                { pid::stackSize,      1.0f },
                { pid::granSample,     4.0f },
                { pid::granDensity,    0.85f },
                { pid::granSizeMs,     45.0f },
                { pid::granPitch,      2.0f },
                { pid::granScatter,    0.8f },
                { pid::granSpray,      0.75f },
                { pid::granMix,        0.95f },
                { pid::filterCutoff,   3500.0f },
                { pid::filterReso,     1.4f },
                { pid::attack,         0.2f },
                { pid::release,        2.4f },
                { pid::spatialSpreadAz, 300.0f },
                { pid::spatialSpreadEl, 65.0f },
                { pid::erRoomSize,     10.0f },
                { pid::erMix,          0.3f },
                { pid::revDecay,       3.4f },
                { pid::revWetDry,      0.36f },
                { pid::gain,           -8.0f },
            }},
        };
        return table;
    }
} // namespace

// ── PresetManager ──────────────────────────────────────────────────────────

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& a)
    : apvts (a)
{
    refresh();
}

void PresetManager::refresh()
{
    entries.clear();
    const auto& table = factoryTable();

    entries.reserve (table.size() + 8);
    for (int i = 0; i < static_cast<int> (table.size()); ++i)
        entries.push_back ({ juce::String (table[static_cast<std::size_t> (i)].name),
                             /*isFactory*/ true, i, juce::File {} });

    const auto dir = getUserPresetDirectory();
    if (dir.isDirectory())
    {
        juce::Array<juce::File> files;
        dir.findChildFiles (files, juce::File::findFiles, false,
                            juce::String ("*") + kFileExtension);
        files.sort();

        for (const auto& f : files)
            entries.push_back ({ f.getFileNameWithoutExtension(),
                                 /*isFactory*/ false, -1, f });
    }

    // Keep the current selection inside the new range. If a user preset
    // was deleted out from under us, fall back to Init.
    int cur = currentIndex.load (std::memory_order_relaxed);
    if (cur < 0 || cur >= static_cast<int> (entries.size()))
        cur = 0;
    currentIndex.store (cur, std::memory_order_relaxed);

    sendChangeMessage();
}

const PresetManager::Entry& PresetManager::getPreset (int index) const
{
    jassert (index >= 0 && index < static_cast<int> (entries.size()));
    return entries.at (static_cast<std::size_t> (index));
}

juce::String PresetManager::getCurrentName() const
{
    const int idx = currentIndex.load (std::memory_order_relaxed);
    if (idx < 0 || idx >= static_cast<int> (entries.size()))
        return "Init";
    return entries[static_cast<std::size_t> (idx)].name;
}

bool PresetManager::loadPreset (int index)
{
    if (index < 0 || index >= static_cast<int> (entries.size()))
        return false;

    const auto& e = entries[static_cast<std::size_t> (index)];
    bool ok = false;
    if (e.isFactory)
    {
        applyFactoryPreset (e.factoryIndex);
        ok = true;
    }
    else
    {
        ok = loadUserPresetFile (e.file);
    }

    if (ok)
    {
        currentIndex.store (index, std::memory_order_relaxed);
        sendChangeMessage();
    }
    return ok;
}

void PresetManager::loadNext()
{
    const int n = static_cast<int> (entries.size());
    if (n == 0) return;
    int next = currentIndex.load (std::memory_order_relaxed) + 1;
    if (next >= n) next = 0;
    loadPreset (next);
}

void PresetManager::loadPrevious()
{
    const int n = static_cast<int> (entries.size());
    if (n == 0) return;
    int prev = currentIndex.load (std::memory_order_relaxed) - 1;
    if (prev < 0) prev = n - 1;
    loadPreset (prev);
}

void PresetManager::loadInit()
{
    resetAllParametersToDefaults();
    currentIndex.store (0, std::memory_order_relaxed);
    sendChangeMessage();
}

bool PresetManager::saveUserPreset (const juce::String& rawName)
{
    auto name = rawName.trim();
    if (name.isEmpty())
        return false;

    // Strip any path separators a user might paste in — we never want a
    // preset name to escape the presets directory.
    name = name.replaceCharacters ("/\\:", "___");

    auto dir = getUserPresetDirectory();
    if (! dir.exists())
    {
        const auto result = dir.createDirectory();
        if (result.failed())
            return false;
    }

    auto file = dir.getChildFile (name + kFileExtension);

    const auto state = apvts.copyState();
    if (! state.isValid())
        return false;

    const auto xml = state.createXml();
    if (xml == nullptr)
        return false;

    if (! xml->writeTo (file))
        return false;

    refresh();

    // Re-select the file we just wrote.
    for (int i = 0; i < static_cast<int> (entries.size()); ++i)
    {
        if (! entries[static_cast<std::size_t> (i)].isFactory
            && entries[static_cast<std::size_t> (i)].file == file)
        {
            currentIndex.store (i, std::memory_order_relaxed);
            break;
        }
    }

    sendChangeMessage();
    return true;
}

juce::File PresetManager::getUserPresetDirectory() const
{
   #if JUCE_MAC
    return juce::File::getSpecialLocation (juce::File::userMusicDirectory)
        .getChildFile ("Binaural Jungle")
        .getChildFile ("Binaural Jungle Forge")
        .getChildFile ("Presets");
   #else
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Binaural Jungle")
        .getChildFile ("Binaural Jungle Forge")
        .getChildFile ("Presets");
   #endif
}

int PresetManager::getNumFactoryPresets()
{
    return static_cast<int> (factoryTable().size());
}

juce::String PresetManager::getFactoryPresetName (int factoryIndex)
{
    const auto& table = factoryTable();
    if (factoryIndex < 0 || factoryIndex >= static_cast<int> (table.size()))
        return {};
    return table[static_cast<std::size_t> (factoryIndex)].name;
}

void PresetManager::applyFactoryPreset (int factoryIndex)
{
    const auto& table = factoryTable();
    if (factoryIndex < 0 || factoryIndex >= static_cast<int> (table.size()))
        return;

    // Reset first so every preset is reproducible regardless of where we
    // arrived from. Any parameter the preset omits inherits its layout
    // default — that's the contract the factory table assumes.
    resetAllParametersToDefaults();

    const auto& preset = table[static_cast<std::size_t> (factoryIndex)];
    for (const auto& [id, rawValue] : preset.overrides)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (rawValue));
    }
}

bool PresetManager::loadUserPresetFile (const juce::File& file)
{
    if (! file.existsAsFile())
        return false;

    auto xml = juce::parseXML (file);
    if (xml == nullptr)
        return false;

    if (! xml->hasTagName (apvts.state.getType()))
        return false;

    apvts.replaceState (juce::ValueTree::fromXml (*xml));
    return true;
}

void PresetManager::resetAllParametersToDefaults()
{
    for (int i = 0; i < pid::kCount; ++i)
    {
        if (auto* p = apvts.getParameter (pid::kAll[i]))
            p->setValueNotifyingHost (p->getDefaultValue());
    }
}

} // namespace bjf
