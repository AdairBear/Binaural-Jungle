#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

#include <atomic>
#include <vector>

namespace bjf
{

// Owns the preset list (factory + user) and is the only thing in the plugin
// allowed to write APVTS state from outside the audio thread. Factory presets
// are baked into the binary as parameter-override tables; user presets live
// on disk as XML next to the user's host preferences.
//
// The class is a juce::ChangeBroadcaster so the GUI (preset name display,
// up/down arrows) can refresh whenever the current selection moves.
class PresetManager final : public juce::ChangeBroadcaster
{
public:
    struct Entry
    {
        juce::String name;
        bool         isFactory     { true };
        int          factoryIndex  { -1 };
        juce::File   file          {};
    };

    explicit PresetManager (juce::AudioProcessorValueTreeState& apvts);

    // Rebuilds the entry list: factory presets first, then any *.bjfpreset
    // files found under getUserPresetDirectory(). Cheap to call; the GUI
    // pings this after a save.
    void refresh();

    int  getNumPresets()   const noexcept { return static_cast<int> (entries.size()); }
    int  getCurrentIndex() const noexcept { return currentIndex.load (std::memory_order_relaxed); }
    const Entry& getPreset (int index) const;
    juce::String getCurrentName() const;

    // Applies a preset to the APVTS. Index is the position in the unified
    // factory+user list (see getPreset). Returns true on success.
    bool loadPreset (int index);
    void loadNext();
    void loadPrevious();

    // Re-applies an "Init" patch: every APVTS parameter back to its default.
    void loadInit();

    // Writes the current APVTS state to <userDir>/<name>.bjfpreset. Overwrites
    // any existing file with the same name. Triggers a refresh + selection of
    // the new file. Returns true on success.
    bool saveUserPreset (const juce::String& name);

    // ~/Music/Binaural Jungle/Binaural Jungle Forge/Presets on macOS;
    // ~/Documents/... on other platforms. Created on first save if missing.
    juce::File getUserPresetDirectory() const;

    static constexpr const char* kFileExtension = ".bjfpreset";

    static int          getNumFactoryPresets();
    static juce::String getFactoryPresetName (int factoryIndex);

private:
    void applyFactoryPreset (int factoryIndex);
    bool loadUserPresetFile (const juce::File& file);
    void resetAllParametersToDefaults();

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<Entry> entries;
    std::atomic<int>   currentIndex { 0 };
};

} // namespace bjf
