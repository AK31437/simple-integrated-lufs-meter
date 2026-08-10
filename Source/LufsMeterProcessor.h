#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "LufsCalculator.h"

class LufsMeterProcessor : public juce::AudioProcessor
{
public:
    LufsMeterProcessor();
    ~LufsMeterProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Simple Integrated LUFS Meter"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    float getIntegratedLoudness() const;
    void requestReset();

private:
    LufsCalculator calculator_;
    std::atomic<bool> resetRequested_{false};
    std::atomic<float> integratedLoudness_{-300.0f};
    bool wasPlaying_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LufsMeterProcessor)
};
