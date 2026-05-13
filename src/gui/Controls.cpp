#include "Controls.h"

#include <cmath>

namespace bjf::gui
{

namespace paintutil
{
    void fillBackground (juce::Graphics& g, juce::Rectangle<float> bounds,
                         juce::Colour fill)
    {
        g.setColour (fill);
        g.fillRoundedRectangle (bounds, theme::panelCornerRadius);
        g.setColour (theme::border);
        g.drawRoundedRectangle (bounds, theme::panelCornerRadius, 1.0f);
    }

    void drawAccentBar (juce::Graphics& g, juce::Rectangle<float> headerArea,
                        juce::Colour accent)
    {
        const float h = theme::accentBarHeight;
        const auto bar = headerArea.removeFromTop (h);
        g.setColour (accent);
        g.fillRect (bar);
    }
}

// ── Panel ─────────────────────────────────────────────────────────────────
namespace
{
    constexpr int headerH = 28;
}

Panel::Panel (const juce::String& titleText, juce::Colour accentColour)
    : title (titleText), accent (accentColour)
{
    setInterceptsMouseClicks (false, true);
}

juce::Rectangle<int> Panel::getContentBounds() const noexcept
{
    return getLocalBounds().withTrimmedTop (headerH + 4).reduced (10, 6);
}

void Panel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    paintutil::fillBackground (g, bounds, theme::panel);

    auto header = bounds.removeFromTop (static_cast<float> (headerH));
    paintutil::drawAccentBar (g, header, accent);

    auto textArea = header.withTrimmedTop (theme::accentBarHeight).reduced (12.0f, 0.0f);
    g.setColour (theme::textPrimary);
    g.setFont (juce::Font (juce::FontOptions ("Menlo", 11.5f, juce::Font::plain)));
    g.drawText (title.toUpperCase(), textArea.toNearestInt(),
                juce::Justification::centredLeft);

    // Subtle horizontal divider under the header.
    g.setColour (theme::border);
    g.fillRect (juce::Rectangle<float> (
        bounds.getX() + 1.0f, bounds.getY() - 0.5f,
        bounds.getWidth() - 2.0f, 1.0f));
}

// ── Knob ─────────────────────────────────────────────────────────────────
Knob::Knob (const juce::String& titleText, juce::Colour accentColour)
{
    title.setText (titleText.toUpperCase(), juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centred);
    title.setColour (juce::Label::textColourId, theme::textSecondary);
    title.setFont (juce::Font (juce::FontOptions ("Menlo", 9.5f, juce::Font::plain)));
    addAndMakeVisible (title);

    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters (theme::rotaryStartRadians,
                                theme::rotaryEndRadians, true);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 72, 14);
    slider.setColour (juce::Slider::rotarySliderFillColourId,    accentColour);
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, theme::track);
    slider.setColour (juce::Slider::textBoxTextColourId,         theme::textPrimary);
    slider.setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    addAndMakeVisible (slider);
}

void Knob::resized()
{
    auto b = getLocalBounds();
    title.setBounds (b.removeFromTop (12));
    slider.setBounds (b);
}

// ── HSlider ──────────────────────────────────────────────────────────────
HSlider::HSlider (const juce::String& titleText, juce::Colour accentColour)
{
    title.setText (titleText.toUpperCase(), juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centredLeft);
    title.setColour (juce::Label::textColourId, theme::textSecondary);
    title.setFont (juce::Font (juce::FontOptions ("Menlo", 9.5f, juce::Font::plain)));
    addAndMakeVisible (title);

    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, true, 56, 16);
    slider.setColour (juce::Slider::trackColourId,             accentColour);
    slider.setColour (juce::Slider::textBoxTextColourId,       theme::textPrimary);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    addAndMakeVisible (slider);
}

void HSlider::resized()
{
    auto b = getLocalBounds();
    title.setBounds (b.removeFromTop (12));
    slider.setBounds (b.reduced (0, 2));
}

// ── ChipToggle ───────────────────────────────────────────────────────────
ChipToggle::ChipToggle (juce::AudioProcessorValueTreeState& apvts_,
                        const juce::String& parameterID,
                        juce::Colour accentColour)
    : apvts (apvts_), paramID (parameterID), accent (accentColour)
{
    param = apvts.getParameter (parameterID);
    jassert (param != nullptr);
    if (param == nullptr)
        return;

    auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param);
    jassert (choice != nullptr);
    if (choice == nullptr)
        return;

    chips.reserve (static_cast<std::size_t> (choice->choices.size()));
    for (int i = 0; i < choice->choices.size(); ++i)
    {
        auto chip = std::make_unique<juce::TextButton> (choice->choices[i]);
        chip->setClickingTogglesState (true);
        chip->setRadioGroupId (1, juce::dontSendNotification);
        chip->setColour (juce::TextButton::buttonOnColourId, accent);
        chip->setColour (juce::TextButton::textColourOnId,   theme::textPrimary);
        chip->setColour (juce::TextButton::textColourOffId,  theme::textSecondary);
        const int index = i;
        chip->onClick = [this, index] { onChipClicked (index); };
        addAndMakeVisible (*chip);
        chips.push_back (std::move (chip));
    }

    // ParameterAttachment provides the parameter-change callback in a
    // thread-safe way. The lambda runs on the message thread.
    attachment = std::make_unique<juce::ParameterAttachment> (
        *param,
        [this] (float /*newValue*/) { refreshFromParameter(); },
        nullptr);
    attachment->sendInitialUpdate();
}

void ChipToggle::onChipClicked (int index)
{
    if (attachment == nullptr || param == nullptr)
        return;

    auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param);
    if (choice == nullptr || choice->choices.size() <= 1)
        return;

    const float normalised = static_cast<float> (index)
                           / static_cast<float> (choice->choices.size() - 1);
    attachment->setValueAsCompleteGesture (param->convertFrom0to1 (normalised));
}

void ChipToggle::refreshFromParameter()
{
    auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param);
    if (choice == nullptr)
        return;

    const int idx = choice->getIndex();
    for (std::size_t i = 0; i < chips.size(); ++i)
        chips[i]->setToggleState (static_cast<int> (i) == idx,
                                  juce::dontSendNotification);
}

void ChipToggle::resized()
{
    if (chips.empty())
        return;

    auto bounds = getLocalBounds();
    const int gap = 4;
    const int totalGap = gap * (static_cast<int> (chips.size()) - 1);
    const int each = (bounds.getWidth() - totalGap) / static_cast<int> (chips.size());

    for (std::size_t i = 0; i < chips.size(); ++i)
    {
        chips[i]->setBounds (bounds.removeFromLeft (each));
        if (i + 1 < chips.size())
            bounds.removeFromLeft (gap);
    }
}

// ── LevelMeter ───────────────────────────────────────────────────────────
// std::atomic<float> is not move-assignable, which forbids
// `vector = vector(size)` in the constructor body. Direct-initialize via
// the member init list instead so each atomic is value-constructed in place.
LevelMeter::LevelMeter (int numChannels)
    : targetLevels (static_cast<std::size_t> (juce::jmax (1, numChannels))),
      displayLevels (targetLevels.size(), 0.0f)
{
    for (auto& a : targetLevels) a.store (0.0f, std::memory_order_relaxed);
    startTimerHz (30);
}

LevelMeter::~LevelMeter() { stopTimer(); }

void LevelMeter::setLevel (int channel, float linearPeak) noexcept
{
    if (channel < 0 || static_cast<std::size_t> (channel) >= targetLevels.size())
        return;
    auto& slot = targetLevels[static_cast<std::size_t> (channel)];
    const auto prev = slot.load (std::memory_order_relaxed);
    if (linearPeak > prev)
        slot.store (linearPeak, std::memory_order_relaxed);
}

void LevelMeter::timerCallback()
{
    bool dirty = false;
    constexpr float decayPerTick = 0.92f;
    for (std::size_t i = 0; i < displayLevels.size(); ++i)
    {
        const auto target = targetLevels[i].exchange (0.0f, std::memory_order_relaxed);
        const auto next = juce::jmax (target, displayLevels[i] * decayPerTick);
        if (! juce::approximatelyEqual (next, displayLevels[i]))
        {
            displayLevels[i] = next;
            dirty = true;
        }
    }
    if (dirty)
        repaint();
}

void LevelMeter::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (theme::track);
    g.fillRoundedRectangle (bounds, 3.0f);

    if (displayLevels.empty())
        return;

    const float chW   = bounds.getWidth() / static_cast<float> (displayLevels.size());
    const float pad   = 2.0f;

    for (std::size_t i = 0; i < displayLevels.size(); ++i)
    {
        const auto col = juce::Rectangle<float> (
            bounds.getX() + static_cast<float> (i) * chW + pad,
            bounds.getY() + pad,
            chW - 2.0f * pad,
            bounds.getHeight() - 2.0f * pad);

        // Convert linear peak to a 0..1 display height using a soft
        // dB-style mapping so quiet content still moves the meter.
        const float linear = juce::jlimit (0.0f, 1.0f, displayLevels[i]);
        const float h01    = std::pow (linear, 0.5f);
        const float fillH  = col.getHeight() * h01;

        const auto fillRect = col.withTrimmedTop (col.getHeight() - fillH);

        // Phosphor accent fading to a warning tone as the meter approaches 1.
        const float warn = juce::jmax (0.0f, (linear - 0.8f) / 0.2f);
        const auto col1 = theme::phosphor;
        const auto col2 = juce::Colour (0xffEE8B5C);
        const auto fill = col1.interpolatedWith (col2, juce::jlimit (0.0f, 1.0f, warn));

        g.setColour (fill);
        g.fillRoundedRectangle (fillRect, 2.0f);
    }
}

} // namespace bjf::gui
