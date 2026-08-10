#include "LufsMeterProcessor.h"
#include "LufsMeterEditor.h"

LufsMeterProcessor::LufsMeterProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

LufsMeterProcessor::~LufsMeterProcessor() {}

void LufsMeterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    calculator_.prepare(sampleRate, getTotalNumInputChannels());
}

void LufsMeterProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (resetRequested_.exchange(false))
        calculator_.reset();

    bool isPlaying = true;
    if (auto* ph = getPlayHead())
    {
        auto pos = ph->getPosition();
        if (pos.hasValue())
            isPlaying = pos->getIsPlaying();
    }

    if (isPlaying && !wasPlaying_)
        calculator_.reset();

    wasPlaying_ = isPlaying;

    if (isPlaying)
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        std::vector<float*> channelPtrs(static_cast<size_t>(numChannels));
        for (int ch = 0; ch < numChannels; ++ch)
            channelPtrs[static_cast<size_t>(ch)] = buffer.getWritePointer(ch);

        calculator_.processBlock(channelPtrs.data(), numChannels, numSamples);
        integratedLoudness_.store(calculator_.getIntegratedLoudness(), std::memory_order_relaxed);
    }
}

float LufsMeterProcessor::getIntegratedLoudness() const
{
    return integratedLoudness_.load(std::memory_order_relaxed);
}

void LufsMeterProcessor::requestReset()
{
    resetRequested_.store(true, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* LufsMeterProcessor::createEditor()
{
    return new LufsMeterEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LufsMeterProcessor();
}
