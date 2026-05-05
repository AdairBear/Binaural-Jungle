#include "PluginEditor.h"

namespace bjf
{
namespace
{
    constexpr juce::uint32 bgColour     = 0xff1a1a2e;
    constexpr juce::uint32 panelColour  = 0xff232336;
    constexpr juce::uint32 trackColour  = 0xff2a2a3e;
    constexpr juce::uint32 textColour   = 0xffe5e5f0;
    constexpr juce::uint32 mutedColour  = 0xff7a7a96;
    constexpr juce::uint32 purpleAccent = 0xff7c3aed;
    constexpr juce::uint32 blueAccent   = 0xff3b82f6;
    constexpr juce::uint32 greenAccent  = 0xff10b981;

    void styleHeader (juce::Label& l)
    {
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colour (textColour));
        l.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    }

    void styleSubLabel (juce::Label& l)
    {
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colour (mutedColour));
        l.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l.setFont (juce::FontOptions (10.0f));
    }
} // namespace

// ─── BJFLookAndFeel ─────────────────────────────────────────────────────────

BJFLookAndFeel::BJFLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (purpleAccent));
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (trackColour));
    setColour (juce::Slider::textBoxTextColourId,         juce::Colour (textColour));
    setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);

    setColour (juce::ComboBox::backgroundColourId, juce::Colour (panelColour));
    setColour (juce::ComboBox::outlineColourId,    juce::Colour (trackColour));
    setColour (juce::ComboBox::textColourId,       juce::Colour (textColour));
    setColour (juce::ComboBox::arrowColourId,      juce::Colour (purpleAccent));
    setColour (juce::ComboBox::buttonColourId,     juce::Colour (panelColour));

    setColour (juce::PopupMenu::backgroundColourId,            juce::Colour (panelColour));
    setColour (juce::PopupMenu::textColourId,                  juce::Colour (textColour));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (purpleAccent));
    setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colours::white);
}

void BJFLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float sliderPos,
                                       float startAngle, float endAngle,
                                       juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.0f);
    const auto centre  = bounds.getCentre();
    const auto radius  = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto arcR    = radius - 4.0f;
    if (arcR <= 1.0f)
        return;

    const auto fill    = slider.findColour (juce::Slider::rotarySliderFillColourId);
    const auto track   = slider.findColour (juce::Slider::rotarySliderOutlineColourId);

    juce::Path bg;
    bg.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                      startAngle, endAngle, true);
    g.setColour (track);
    g.strokePath (bg, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    const auto angle = startAngle + sliderPos * (endAngle - startAngle);
    juce::Path active;
    active.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                          startAngle, angle, true);
    g.setColour (fill);
    g.strokePath (active, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));

    const auto innerR = arcR - 6.0f;
    if (innerR > 1.0f)
    {
        g.setColour (juce::Colour (panelColour));
        g.fillEllipse (juce::Rectangle<float> (innerR * 2.0f, innerR * 2.0f).withCentre (centre));

        juce::Path pointer;
        const float thickness = 2.5f;
        pointer.addRectangle (-thickness * 0.5f,
                              -innerR * 0.95f,
                              thickness,
                              innerR * 0.55f);
        pointer.applyTransform (juce::AffineTransform::rotation (angle)
                                    .translated (centre.x, centre.y));
        g.setColour (fill);
        g.fillPath (pointer);
    }
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

    juce::Path arrow;
    const float aw = 8.0f, ah = 4.0f;
    const auto cx = bounds.getRight() - aw;
    const auto cy = bounds.getCentreY();
    arrow.addTriangle (cx - aw * 0.5f, cy - ah * 0.5f,
                       cx + aw * 0.5f, cy - ah * 0.5f,
                       cx,             cy + ah * 0.5f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (arrow);
}

void BJFLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 0, box.getWidth() - 24, box.getHeight());
    label.setFont (getComboBoxFont (box));
    label.setJustificationType (juce::Justification::centredLeft);
}

juce::Font BJFLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (12.0f));
}

juce::Font BJFLookAndFeel::getLabelFont (juce::Label& l)
{
    return l.getFont();
}

// ─── LabeledKnob ────────────────────────────────────────────────────────────

LabeledKnob::LabeledKnob (const juce::String& titleText, juce::Colour accentColour)
{
    title.setText (titleText, juce::dontSendNotification);
    styleSubLabel (title);
    addAndMakeVisible (title);

    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 64, 14);
    slider.setColour (juce::Slider::rotarySliderFillColourId,    accentColour);
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (trackColour));
    slider.setColour (juce::Slider::textBoxTextColourId,         juce::Colour (textColour));
    slider.setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    addAndMakeVisible (slider);
}

void LabeledKnob::resized()
{
    auto b = getLocalBounds();
    title.setBounds (b.removeFromTop (14));
    slider.setBounds (b);
}

// ─── LabeledChoice ──────────────────────────────────────────────────────────

LabeledChoice::LabeledChoice (const juce::String& titleText)
{
    title.setText (titleText, juce::dontSendNotification);
    styleSubLabel (title);
    addAndMakeVisible (title);
    addAndMakeVisible (combo);
}

void LabeledChoice::resized()
{
    auto b = getLocalBounds();
    title.setBounds (b.removeFromTop (14));
    combo.setBounds (b.reduced (4, 4));
}

// ─── XYPad ──────────────────────────────────────────────────────────────────

XYPad::XYPad (juce::AudioProcessorValueTreeState& apvts,
              const juce::String& xParamID, const juce::String& yParamID,
              juce::Colour accentColour)
    : accent (accentColour)
{
    xParam = apvts.getParameter (xParamID);
    yParam = apvts.getParameter (yParamID);
    if (xParam != nullptr) lastX = xParam->getValue();
    if (yParam != nullptr) lastY = yParam->getValue();
    startTimerHz (30);
}

XYPad::~XYPad()
{
    stopTimer();
}

void XYPad::timerCallback()
{
    if (xParam == nullptr || yParam == nullptr)
        return;

    const float x = xParam->getValue();
    const float y = yParam->getValue();
    if (! juce::approximatelyEqual (x, lastX) || ! juce::approximatelyEqual (y, lastY))
    {
        lastX = x;
        lastY = y;
        repaint();
    }
}

void XYPad::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    g.setColour (juce::Colour (panelColour));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colour (trackColour));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    g.setColour (juce::Colour (trackColour));
    g.drawLine (bounds.getX(), bounds.getCentreY(),
                bounds.getRight(), bounds.getCentreY(), 1.0f);
    g.drawLine (bounds.getCentreX(), bounds.getY(),
                bounds.getCentreX(), bounds.getBottom(), 1.0f);

    if (xParam == nullptr || yParam == nullptr)
        return;

    const float nx = xParam->getValue();
    const float ny = yParam->getValue();
    const float dotX = bounds.getX() + nx * bounds.getWidth();
    const float dotY = bounds.getY() + (1.0f - ny) * bounds.getHeight();

    g.setColour (accent.withAlpha (0.25f));
    g.fillEllipse (dotX - 12.0f, dotY - 12.0f, 24.0f, 24.0f);
    g.setColour (accent);
    g.fillEllipse (dotX - 5.0f, dotY - 5.0f, 10.0f, 10.0f);
}

void XYPad::mouseDown (const juce::MouseEvent& e) { writeFromMouse (e); }
void XYPad::mouseDrag (const juce::MouseEvent& e) { writeFromMouse (e); }

void XYPad::writeFromMouse (const juce::MouseEvent& e)
{
    if (xParam == nullptr || yParam == nullptr)
        return;

    const auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    const float w = juce::jmax (1.0f, bounds.getWidth());
    const float h = juce::jmax (1.0f, bounds.getHeight());
    const float nx = juce::jlimit (0.0f, 1.0f,
                                   (static_cast<float> (e.x) - bounds.getX()) / w);
    const float ny = juce::jlimit (0.0f, 1.0f,
                                   1.0f - (static_cast<float> (e.y) - bounds.getY()) / h);
    xParam->setValueNotifyingHost (nx);
    yParam->setValueNotifyingHost (ny);
    lastX = nx;
    lastY = ny;
    repaint();
}

// ─── BinauralJungleForgeEditor ──────────────────────────────────────────────

BinauralJungleForgeEditor::BinauralJungleForgeEditor (BinauralJungleForgeProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
      stackSizeKnob ("Stack",     juce::Colour (purpleAccent)),
      detuneKnob    ("Detune",    juce::Colour (purpleAccent)),
      cutoffKnob    ("Cutoff",    juce::Colour (blueAccent)),
      resonanceKnob ("Resonance", juce::Colour (blueAccent)),
      attackKnob    ("Attack",    juce::Colour (greenAccent)),
      decayKnob     ("Decay",     juce::Colour (greenAccent)),
      sustainKnob   ("Sustain",   juce::Colour (greenAccent)),
      releaseKnob   ("Release",   juce::Colour (greenAccent)),
      spatialPad    (p.apvts, "spatial_az", "spatial_el",
                     juce::Colour (purpleAccent)),
      spreadAzKnob  ("Spread Az", juce::Colour (purpleAccent)),
      spreadElKnob  ("Spread El", juce::Colour (purpleAccent)),
      gainKnob      ("Gain",      juce::Colour (blueAccent))
{
    setLookAndFeel (&lnf);

    titleLabel.setText ("BINAURAL JUNGLE FORGE", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (textColour));
    titleLabel.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    auto initSection = [this] (juce::Label& l, const juce::String& text)
    {
        l.setText (text, juce::dontSendNotification);
        styleHeader (l);
        addAndMakeVisible (l);
    };

    initSection (oscHeader,     "OSCILLATORS");
    initSection (filterHeader,  "FILTER");
    initSection (envHeader,     "ENVELOPE");
    initSection (spatialHeader, "SPATIAL");
    initSection (masterHeader,  "MASTER");

    waveformChoice.getCombo().addItem ("Saw",    1);
    waveformChoice.getCombo().addItem ("Square", 2);
    addAndMakeVisible (waveformChoice);
    addAndMakeVisible (stackSizeKnob);
    addAndMakeVisible (detuneKnob);

    filterTypeChoice.getCombo().addItem ("Low Pass",  1);
    filterTypeChoice.getCombo().addItem ("Band Pass", 2);
    filterTypeChoice.getCombo().addItem ("High Pass", 3);
    addAndMakeVisible (filterTypeChoice);
    addAndMakeVisible (cutoffKnob);
    addAndMakeVisible (resonanceKnob);

    addAndMakeVisible (attackKnob);
    addAndMakeVisible (decayKnob);
    addAndMakeVisible (sustainKnob);
    addAndMakeVisible (releaseKnob);

    addAndMakeVisible (spatialPad);
    addAndMakeVisible (spreadAzKnob);
    addAndMakeVisible (spreadElKnob);

    addAndMakeVisible (gainKnob);

    auto& a = processorRef.apvts;
    waveformAtt   = std::make_unique<ComboBoxAttachment> (a, "waveform",          waveformChoice.getCombo());
    stackSizeAtt  = std::make_unique<SliderAttachment>   (a, "osc_stack_size",    stackSizeKnob.getSlider());
    detuneAtt     = std::make_unique<SliderAttachment>   (a, "osc_detune_cents",  detuneKnob.getSlider());
    filterTypeAtt = std::make_unique<ComboBoxAttachment> (a, "filter_type",       filterTypeChoice.getCombo());
    cutoffAtt     = std::make_unique<SliderAttachment>   (a, "filter_cutoff",     cutoffKnob.getSlider());
    resonanceAtt  = std::make_unique<SliderAttachment>   (a, "filter_resonance",  resonanceKnob.getSlider());
    attackAtt     = std::make_unique<SliderAttachment>   (a, "amp_attack",        attackKnob.getSlider());
    decayAtt      = std::make_unique<SliderAttachment>   (a, "amp_decay",         decayKnob.getSlider());
    sustainAtt    = std::make_unique<SliderAttachment>   (a, "amp_sustain",       sustainKnob.getSlider());
    releaseAtt    = std::make_unique<SliderAttachment>   (a, "amp_release",       releaseKnob.getSlider());
    spreadAzAtt   = std::make_unique<SliderAttachment>   (a, "spatial_spread_az", spreadAzKnob.getSlider());
    spreadElAtt   = std::make_unique<SliderAttachment>   (a, "spatial_spread_el", spreadElKnob.getSlider());
    gainAtt       = std::make_unique<SliderAttachment>   (a, "gain",              gainKnob.getSlider());

    setSize (920, 420);
}

BinauralJungleForgeEditor::~BinauralJungleForgeEditor()
{
    setLookAndFeel (nullptr);
}

namespace
{
    struct Lanes
    {
        juce::Rectangle<int> osc, filter, env, spatial, master;
    };

    Lanes carveLanes (juce::Rectangle<int> body)
    {
        Lanes out;
        body.reduce (8, 8);
        out.osc    = body.removeFromLeft  (180);  body.removeFromLeft  (8);
        out.filter = body.removeFromLeft  (180);  body.removeFromLeft  (8);
        out.env    = body.removeFromLeft  (200);  body.removeFromLeft  (8);
        out.master = body.removeFromRight (80);   body.removeFromRight (8);
        out.spatial = body;
        return out;
    }
} // namespace

void BinauralJungleForgeEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (bgColour));

    auto titleBar = getLocalBounds().removeFromTop (40);
    g.setColour (juce::Colour (purpleAccent));
    g.fillRect (titleBar.removeFromBottom (1));

    auto bodyBounds = getLocalBounds().withTrimmedTop (40);
    const auto lanes = carveLanes (bodyBounds);

    auto drawPanel = [&g] (juce::Rectangle<int> r, juce::Colour accent)
    {
        const auto fr = r.toFloat();
        g.setColour (juce::Colour (panelColour));
        g.fillRoundedRectangle (fr, 6.0f);
        g.setColour (accent.withAlpha (0.35f));
        g.drawRoundedRectangle (fr, 6.0f, 1.0f);
    };

    drawPanel (lanes.osc,     juce::Colour (purpleAccent));
    drawPanel (lanes.filter,  juce::Colour (blueAccent));
    drawPanel (lanes.env,     juce::Colour (greenAccent));
    drawPanel (lanes.spatial, juce::Colour (purpleAccent));
    drawPanel (lanes.master,  juce::Colour (blueAccent));
}

void BinauralJungleForgeEditor::resized()
{
    auto bounds = getLocalBounds();
    auto titleBar = bounds.removeFromTop (40);
    titleLabel.setBounds (titleBar.reduced (16, 0));

    const auto lanes = carveLanes (bounds);

    constexpr int headerH = 22;
    constexpr int knobH   = 92;
    constexpr int comboH  = 36;
    constexpr int gap     = 4;

    auto layoutHeader = [] (juce::Rectangle<int> lane, juce::Label& h)
    {
        h.setBounds (lane.removeFromTop (headerH));
        return lane.withTrimmedTop (headerH + gap);
    };

    {
        auto lane = lanes.osc.reduced (8, 8);
        lane = layoutHeader (lane, oscHeader);
        waveformChoice.setBounds (lane.removeFromTop (comboH));
        lane.removeFromTop (gap);
        auto row = lane.removeFromTop (knobH);
        const auto each = row.getWidth() / 2;
        stackSizeKnob.setBounds (row.removeFromLeft (each));
        detuneKnob   .setBounds (row);
    }

    {
        auto lane = lanes.filter.reduced (8, 8);
        lane = layoutHeader (lane, filterHeader);
        filterTypeChoice.setBounds (lane.removeFromTop (comboH));
        lane.removeFromTop (gap);
        auto row = lane.removeFromTop (knobH);
        const auto each = row.getWidth() / 2;
        cutoffKnob   .setBounds (row.removeFromLeft (each));
        resonanceKnob.setBounds (row);
    }

    {
        auto lane = lanes.env.reduced (8, 8);
        lane = layoutHeader (lane, envHeader);
        auto top = lane.removeFromTop (knobH);
        const auto half = top.getWidth() / 2;
        attackKnob.setBounds (top.removeFromLeft (half));
        decayKnob .setBounds (top);
        lane.removeFromTop (gap);
        auto bot = lane.removeFromTop (knobH);
        sustainKnob.setBounds (bot.removeFromLeft (half));
        releaseKnob.setBounds (bot);
    }

    {
        auto lane = lanes.spatial.reduced (8, 8);
        lane = layoutHeader (lane, spatialHeader);
        const auto padHeight = juce::jmax (60, lane.getHeight() - knobH - gap);
        spatialPad.setBounds (lane.removeFromTop (padHeight));
        lane.removeFromTop (gap);
        auto row = lane.removeFromTop (knobH);
        const auto half = row.getWidth() / 2;
        spreadAzKnob.setBounds (row.removeFromLeft (half));
        spreadElKnob.setBounds (row);
    }

    {
        auto lane = lanes.master.reduced (4, 8);
        lane = layoutHeader (lane, masterHeader);
        gainKnob.setBounds (lane.removeFromTop (knobH));
    }
}

} // namespace bjf
