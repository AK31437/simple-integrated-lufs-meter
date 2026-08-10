#include "LufsMeterEditor.h"
#include "LufsMeterProcessor.h"

ResetButton::ResetButton()
    : juce::Button({})
{
    setTooltip("Reset measurement");
}

void ResetButton::mouseUp(const juce::MouseEvent& event)
{
    clickTime_ = juce::Time::getMillisecondCounterHiRes() * 0.001;
    animating_ = true;
    startTimerHz(kAnimHz);
    Button::mouseUp(event);
}

void ResetButton::timerCallback()
{
    double elapsed = juce::Time::getMillisecondCounterHiRes() * 0.001 - clickTime_;
    if (elapsed >= kAnimDuration)
    {
        animating_ = false;
        stopTimer();
    }
    repaint();
}

void ResetButton::paintButton(juce::Graphics& g, bool isOver, bool isDown)
{
    auto area = getLocalBounds().toFloat();
    auto centre = area.getCentre();
    auto baseRadius = juce::jmin(area.getWidth(), area.getHeight()) * 0.5f - 1.0f;

    // Compute press blend: 0 = normal, 1 = fully pressed
    float pressBlend = 0.0f;
    if (isDown)
        pressBlend = 1.0f;
    else if (animating_)
    {
        double elapsed = juce::Time::getMillisecondCounterHiRes() * 0.001 - clickTime_;
        float t = juce::jlimit(0.0f, 1.0f, static_cast<float>(elapsed / kAnimDuration));
        pressBlend = (1.0f - t) * (1.0f - t); // quadratic ease-out: fast snap, slow settle
    }

    // Circle shrinks slightly when pressed
    float radius = baseRadius - pressBlend * 2.0f;

    // Gradient inverts: convex (light top) → concave (dark top)
    juce::Colour normalTop(0xff3a3a3a),     normalBottom(0xff242424);
    juce::Colour pressedTop(0xff1e1e1e),    pressedBottom(0xff404040);

    if (isOver && pressBlend < 0.5f)
    {
        normalTop   = juce::Colour(0xff5a5a5a);
        normalBottom = juce::Colour(0xff303030);
    }

    juce::Colour topCol    = normalTop.interpolatedWith(pressedTop, pressBlend);
    juce::Colour bottomCol = normalBottom.interpolatedWith(pressedBottom, pressBlend);

    // Main circle
    juce::ColourGradient grad(topCol, centre.x, centre.y - radius * 0.3f,
                              bottomCol, centre.x, centre.y + radius, true);
    g.setGradientFill(grad);
    g.fillEllipse(centre.x - radius, centre.y - radius,
                   radius * 2.0f, radius * 2.0f);

    // Border — slightly more visible when pressed (edge of the "hole")
    float borderAlpha = 0.25f + pressBlend * 0.2f;
    g.setColour(juce::Colour::fromRGBA(255, 255, 255, static_cast<juce::uint8>(borderAlpha * 255)));
    g.drawEllipse(centre.x - radius, centre.y - radius,
                   radius * 2.0f, radius * 2.0f, 1.0f);

    // Top highlight — fades out when pressed (no highlight in a hole)
    {
        float hlAlpha = (1.0f - pressBlend) * (isOver ? 14.0f : 8.0f);
        float hlRadius = radius * 0.65f;
        float hlY = centre.y - radius * 0.35f;
        g.setColour(juce::Colour::fromRGBA(255, 255, 255, static_cast<juce::uint8>(hlAlpha)));
        g.fillEllipse(centre.x - hlRadius, hlY - hlRadius * 0.45f,
                       hlRadius * 2.0f, hlRadius * 0.9f);
    }
}

LufsMeterEditor::LufsMeterEditor(LufsMeterProcessor& p)
    : AudioProcessorEditor(p), processor_(p)
{
    setSize(kWidth, kHeight);

    resetButton_.onClick = [this] { processor_.requestReset(); };
    addAndMakeVisible(resetButton_);

    startTimerHz(30);
}

LufsMeterEditor::~LufsMeterEditor()
{
    stopTimer();
}

void LufsMeterEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff141414));

    const int w = getWidth();

    // Value text
    juce::String valueText;
    if (currentLufs_ <= -299.0f)
        valueText = "-inf";
    else
        valueText = juce::String(currentLufs_, 1);

    juce::Font valueFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(),
                                           52.0f, juce::Font::plain));
    g.setFont(valueFont);
    g.setColour(juce::Colours::white);

    auto valueArea = juce::Rectangle<int>(0, 15, w, 70);
    g.drawFittedText(valueText, valueArea, juce::Justification::centred, 1);

    // Label
    juce::Font labelFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(),
                                           12.0f, juce::Font::plain));
    g.setFont(labelFont);
    g.setColour(juce::Colour(0xff777777));

    auto labelArea = juce::Rectangle<int>(0, 80, w, 20);
    g.drawFittedText("Integrated LUFS", labelArea, juce::Justification::centred, 1);
}

void LufsMeterEditor::resized()
{
    resetButton_.setBounds((getWidth() - kButtonSize) / 2,
                           getHeight() - kButtonSize - 18,
                           kButtonSize, kButtonSize);
}

void LufsMeterEditor::timerCallback()
{
    float newValue = processor_.getIntegratedLoudness();

    if (! initialPaintDone_ || std::abs(newValue - currentLufs_) > 0.05f)
    {
        currentLufs_ = newValue;
        initialPaintDone_ = true;
        repaint();
    }
}
