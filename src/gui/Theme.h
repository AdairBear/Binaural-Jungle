#pragma once

#include <juce_graphics/juce_graphics.h>

// Design tokens for the Binaural Jungle Forge GUI. Everything visual that has
// a "brand value" — a colour, a sizing constant, a corner radius — lives here
// so the palette stays coherent and any later rebrand is a single-file edit.
//
// Numbers come straight from the interactive HTML prototype. They are not
// suggestions; the panels and the LookAndFeel both calibrate to them.

namespace bjf::theme
{
    // ── Surfaces ───────────────────────────────────────────────────────────
    inline const juce::Colour background { 0xff0A0C12 };
    inline const juce::Colour panel      { 0xff0D1017 };
    inline const juce::Colour panelDeep  { 0xff070910 };
    inline const juce::Colour border     { 0xff1F2530 };
    inline const juce::Colour track      { 0xff141821 };

    // ── Text ───────────────────────────────────────────────────────────────
    inline const juce::Colour textPrimary   { 0xffE8DFC6 };
    inline const juce::Colour textSecondary { 0xff6B7280 };
    inline const juce::Colour textMuted     { 0xff444B57 };

    // ── Accents (one per panel family) ─────────────────────────────────────
    inline const juce::Colour indigo   { 0xff6B6BFF }; // Oscillator
    inline const juce::Colour violet   { 0xffA78BFA }; // Granular
    inline const juce::Colour cyan     { 0xff7DD3FC }; // Filter
    inline const juce::Colour cream    { 0xffE8DFC6 }; // Envelope
    inline const juce::Colour phosphor { 0xff6FE3B8 }; // HOA Reverb / meter
    inline const juce::Colour polaris  { 0xffE8DFC6 }; // master / spatial UI

    // ── Geometry ───────────────────────────────────────────────────────────
    inline constexpr float panelCornerRadius = 6.0f;
    inline constexpr float knobCornerRadius  = 999.0f; // fully round
    inline constexpr float accentBarHeight   = 2.0f;

    // ── Knob arc ──────────────────────────────────────────────────────────
    // The HTML prototype draws a knob arc from −135° (left of bottom) to
    // +135° (right of bottom) of vertical. JUCE rotary slider angles are
    // measured clockwise from 12 o'clock, so the equivalent pair is:
    inline constexpr float rotaryStartRadians = -2.35619449f; // −135°
    inline constexpr float rotaryEndRadians   =  2.35619449f; // +135°
    inline constexpr int   rotaryTickCount    = 19;
} // namespace bjf::theme
