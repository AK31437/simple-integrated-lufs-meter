#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

class LufsMeterProcessor;

class ResetButton : public juce::Button, private juce::Timer
{
public:
    ResetButton();

protected:
    void paintButton(juce::Graphics& g, bool isOver, bool isDown) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;

    double clickTime_ = 0.0;
    bool animating_ = false;

    static constexpr double kAnimDuration = 0.25;
    static constexpr int kAnimHz = 60;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResetButton)
};

class LufsMeterEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    LufsMeterEditor(LufsMeterProcessor&);
    ~LufsMeterEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    LufsMeterProcessor& processor_;
    ResetButton resetButton_;
    float currentLufs_ = -300.0f;
    bool initialPaintDone_ = false;

    static constexpr int kWidth = 260;
    static constexpr int kHeight = 160;
    static constexpr int kButtonSize = 28;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LufsMeterEditor)
};
