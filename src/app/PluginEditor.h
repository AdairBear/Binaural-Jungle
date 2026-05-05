#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace bjf
{

class BJFLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    BJFLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int w, int h,
                       bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getLabelFont (juce::Label&) override;
};

class LabeledKnob final : public juce::Component
{
public:
    LabeledKnob (const juce::String& titleText, juce::Colour accentColour);

    juce::Slider& getSlider() noexcept { return slider; }

    void resized() override;

private:
    juce::Label  title;
    juce::Slider slider;
};

class LabeledChoice final : public juce::Component
{
public:
    explicit LabeledChoice (const juce::String& titleText);

    juce::ComboBox& getCombo() noexcept { return combo; }

    void resized() override;

private:
    juce::Label    title;
    juce::ComboBox combo;
};

class XYPad final : public juce::Component, private juce::Timer
{
public:
    XYPad (juce::AudioProcessorValueTreeState& apvts,
           const juce::String& xParamID,
           const juce::String& yParamID,
           juce::Colour accentColour);

    ~XYPad() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void writeFromMouse (const juce::MouseEvent&);

    juce::RangedAudioParameter* xParam { nullptr };
    juce::RangedAudioParameter* yParam { nullptr };
    juce::Colour accent;
    float lastX { 0.5f }, lastY { 0.5f };
};

class BinauralJungleForgeEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BinauralJungleForgeEditor (BinauralJungleForgeProcessor&);
    ~BinauralJungleForgeEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    BinauralJungleForgeProcessor& processorRef;

    BJFLookAndFeel lnf;

    juce::Label titleLabel;
    juce::Label oscHeader, filterHeader, envHeader, spatialHeader, masterHeader;

    LabeledChoice waveformChoice  { "Waveform"  };
    LabeledKnob   stackSizeKnob;
    LabeledKnob   detuneKnob;

    LabeledChoice filterTypeChoice { "Type" };
    LabeledKnob   cutoffKnob;
    LabeledKnob   resonanceKnob;

    LabeledKnob   attackKnob;
    LabeledKnob   decayKnob;
    LabeledKnob   sustainKnob;
    LabeledKnob   releaseKnob;

    XYPad         spatialPad;
    LabeledKnob   spreadAzKnob;
    LabeledKnob   spreadElKnob;

    LabeledKnob   gainKnob;

    std::unique_ptr<ComboBoxAttachment> waveformAtt;
    std::unique_ptr<SliderAttachment>   stackSizeAtt, detuneAtt;
    std::unique_ptr<ComboBoxAttachment> filterTypeAtt;
    std::unique_ptr<SliderAttachment>   cutoffAtt, resonanceAtt;
    std::unique_ptr<SliderAttachment>   attackAtt, decayAtt, sustainAtt, releaseAtt;
    std::unique_ptr<SliderAttachment>   spreadAzAtt, spreadElAtt;
    std::unique_ptr<SliderAttachment>   gainAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauralJungleForgeEditor)
};

} // namespace bjf
