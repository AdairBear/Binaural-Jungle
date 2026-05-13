#include "BJFLookAndFeel.h"

#include "Theme.h"

namespace bjf::gui
{

BJFLookAndFeel::BJFLookAndFeel()
{
    // Slider colours are overridden per-instance by the Knob/HSlider wrappers,
    // but seed sensible global defaults so a raw juce::Slider drawn through
    // this L&F doesn't render as the default JUCE blue.
    setColour (juce::Slider::rotarySliderFillColourId,    theme::indigo);
    setColour (juce::Slider::rotarySliderOutlineColourId, theme::track);
    setColour (juce::Slider::trackColourId,               theme::track);
    setColour (juce::Slider::backgroundColourId,          theme::panel);
    setColour (juce::Slider::thumbColourId,               theme::textPrimary);
    setColour (juce::Slider::textBoxTextColourId,         theme::textPrimary);
    setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);

    setColour (juce::ComboBox::backgroundColourId, theme::track);
    setColour (juce::ComboBox::outlineColourId,    theme::border);
    setColour (juce::ComboBox::textColourId,       theme::textPrimary);
    setColour (juce::ComboBox::arrowColourId,      theme::textSecondary);
    setColour (juce::ComboBox::buttonColourId,     theme::track);

    setColour (juce::PopupMenu::backgroundColourId,            theme::panel);
    setColour (juce::PopupMenu::textColourId,                  theme::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, theme::indigo.withAlpha (0.4f));
    setColour (juce::PopupMenu::highlightedTextColourId,       theme::textPrimary);

    setColour (juce::TextButton::buttonColourId,   theme::track);
    setColour (juce::TextButton::buttonOnColourId, theme::indigo);
    setColour (juce::TextButton::textColourOffId,  theme::textSecondary);
    setColour (juce::TextButton::textColourOnId,   theme::textPrimary);

    setColour (juce::Label::textColourId,       theme::textPrimary);
    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
}

void BJFLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float sliderPos,
                                       float startAngle, float endAngle,
                                       juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.0f);
    const auto centre = bounds.getCentre();
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    if (radius <= 4.0f)
        return;

    const auto accent = slider.findColour (juce::Slider::rotarySliderFillColourId);
    const auto trackC = slider.findColour (juce::Slider::rotarySliderOutlineColourId);

    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // ── Tick ring just outside the body ───────────────────────────────────
    const float tickR1 = radius - 1.0f;
    const float tickR2 = radius - 4.0f;
    g.setColour (theme::border);
    for (int i = 0; i < theme::rotaryTickCount; ++i)
    {
        const float t = i / static_cast<float> (theme::rotaryTickCount - 1);
        const float a = startAngle + t * (endAngle - startAngle);
        // Active ticks ride the accent colour; the rest stay the neutral
        // border tone so the dial gradient is legible at a glance.
        const auto col = (a <= angle + 0.001f) ? accent.withAlpha (0.85f)
                                               : theme::border;
        g.setColour (col);
        const float sinA = std::sin (a);
        const float cosA = std::cos (a);
        const float x1 = centre.x + sinA * tickR2;
        const float y1 = centre.y - cosA * tickR2;
        const float x2 = centre.x + sinA * tickR1;
        const float y2 = centre.y - cosA * tickR1;
        g.drawLine (x1, y1, x2, y2, 1.4f);
    }

    // ── Knob body (radial gradient) ───────────────────────────────────────
    const float bodyR = radius - 7.0f;
    if (bodyR > 0.0f)
    {
        const auto bodyBounds = juce::Rectangle<float> { bodyR * 2.0f, bodyR * 2.0f }
                                    .withCentre (centre);
        juce::ColourGradient grad (theme::panel.brighter (0.15f), centre.x, centre.y - bodyR * 0.6f,
                                   theme::panelDeep,              centre.x, centre.y + bodyR,
                                   false);
        g.setGradientFill (grad);
        g.fillEllipse (bodyBounds);

        g.setColour (theme::border);
        g.drawEllipse (bodyBounds, 1.0f);
    }

    // ── Accent arc on the live half ───────────────────────────────────────
    const float arcR = radius - 6.0f;
    if (arcR > 0.0f)
    {
        juce::Path bg;
        bg.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                          startAngle, endAngle, true);
        g.setColour (trackC.brighter (0.05f));
        g.strokePath (bg, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));

        juce::Path active;
        active.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                              startAngle, angle, true);
        g.setColour (accent);
        g.strokePath (active, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved,
                                                           juce::PathStrokeType::rounded));
    }

    // ── Pointer ───────────────────────────────────────────────────────────
    if (bodyR > 2.0f)
    {
        const float pointerLen = bodyR * 0.78f;
        const float thickness  = 2.2f;
        juce::Path pointer;
        pointer.addRoundedRectangle (-thickness * 0.5f, -pointerLen,
                                      thickness, pointerLen * 0.65f,
                                      thickness * 0.5f);
        pointer.applyTransform (juce::AffineTransform::rotation (angle)
                                    .translated (centre.x, centre.y));
        g.setColour (accent);
        g.fillPath (pointer);

        // Centre cap
        g.setColour (theme::panel.brighter (0.25f));
        const auto cap = juce::Rectangle<float> { 4.0f, 4.0f }.withCentre (centre);
        g.fillEllipse (cap);
    }
}

void BJFLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float sliderPos,
                                       float /*minPos*/, float /*maxPos*/,
                                       juce::Slider::SliderStyle style,
                                       juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearBar)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos,
                                          0.0f, 0.0f, style, slider);
        return;
    }

    const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat();
    const auto trackBounds = bounds.withSizeKeepingCentre (bounds.getWidth(), 4.0f);

    g.setColour (theme::track);
    g.fillRoundedRectangle (trackBounds, 2.0f);

    const auto accent = slider.findColour (juce::Slider::trackColourId);

    const float fillRight = juce::jlimit (trackBounds.getX(), trackBounds.getRight(),
                                          sliderPos);
    const auto fill = trackBounds.withRight (fillRight);
    g.setColour (accent);
    g.fillRoundedRectangle (fill, 2.0f);

    // Five tick marks above the track for slider micro-affordance.
    g.setColour (theme::border);
    constexpr int ticks = 5;
    for (int i = 0; i < ticks; ++i)
    {
        const float t = i / static_cast<float> (ticks - 1);
        const float tx = trackBounds.getX() + t * trackBounds.getWidth();
        g.drawLine (tx, trackBounds.getY() - 6.0f, tx, trackBounds.getY() - 2.0f, 1.0f);
    }

    // Thumb
    const float thumbR = 6.0f;
    juce::Rectangle<float> thumb { thumbR * 2.0f, thumbR * 2.0f };
    thumb.setCentre (fillRight, trackBounds.getCentreY());
    g.setColour (theme::textPrimary);
    g.fillEllipse (thumb);
    g.setColour (theme::panel);
    g.drawEllipse (thumb, 1.0f);
}

void BJFLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h,
                                   bool /*isButtonDown*/,
                                   int /*bx*/, int /*by*/, int /*bw*/, int /*bh*/,
                                   juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<int> (0, 0, w, h).toFloat().reduced (1.0f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    // Disclosure chevron — small, monospace vibe matching the chip labels.
    juce::Path arrow;
    const float aw = 8.0f, ah = 4.0f;
    const auto cx = bounds.getRight() - aw - 4.0f;
    const auto cy = bounds.getCentreY();
    arrow.addTriangle (cx - aw * 0.5f, cy - ah * 0.5f,
                       cx + aw * 0.5f, cy - ah * 0.5f,
                       cx,             cy + ah * 0.5f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (arrow);
}

void BJFLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (10, 0, box.getWidth() - 28, box.getHeight());
    label.setFont (getComboBoxFont (box));
    label.setJustificationType (juce::Justification::centredLeft);
}

juce::Font BJFLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions ("Menlo", 11.5f, juce::Font::plain));
}

juce::Font BJFLookAndFeel::getLabelFont (juce::Label& l) { return l.getFont(); }

void BJFLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& /*backgroundColour*/,
                                           bool isHighlighted, bool /*isDown*/)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const auto on = button.getToggleState();

    const auto bg = on
        ? button.findColour (juce::TextButton::buttonOnColourId).withAlpha (0.18f)
        : theme::track;
    const auto outline = on
        ? button.findColour (juce::TextButton::buttonOnColourId)
        : theme::border;

    g.setColour (bg);
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (outline.withAlpha (isHighlighted ? 0.9f : 0.7f));
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
}

void BJFLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                     bool /*isHighlighted*/, bool /*isDown*/)
{
    g.setFont (juce::Font (juce::FontOptions ("Menlo", 10.5f, juce::Font::plain)));
    const auto colour = button.getToggleState()
        ? button.findColour (juce::TextButton::textColourOnId)
        : button.findColour (juce::TextButton::textColourOffId);
    g.setColour (colour);
    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().reduced (4, 2),
                      juce::Justification::centred, 1);
}

} // namespace bjf::gui
