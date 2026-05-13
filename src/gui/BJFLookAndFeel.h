#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bjf::gui
{

// Custom LookAndFeel for the entire plugin GUI. Holds no per-instance state;
// each component still owns its own colours and labels and the L&F only
// rasterises them. Knob ticks, the accent arc, and the rounded combo
// stripes all live here so the panels can stay layout-only.
class BJFLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    BJFLookAndFeel();

    // ── Rotary slider (the dominant primitive on the right side) ──────────
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    // ── Horizontal linear slider (the bars under panels) ──────────────────
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    // ── ComboBox (compact "dropdown" used for sample/preset selection) ────
    void drawComboBox (juce::Graphics&, int w, int h,
                       bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getLabelFont (juce::Label&) override;

    // ── TextButton (used for chip toggles inside panels) ──────────────────
    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool isMouseOverButton, bool isButtonDown) override;
};

} // namespace bjf::gui
