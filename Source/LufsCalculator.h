#pragma once

#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <cstring>

class LufsCalculator
{
public:
    LufsCalculator() = default;
    ~LufsCalculator() = default;

    void prepare(double sampleRate, int numChannels);
    void processBlock(float* const* channelData, int numChannels, int numSamples);
    float getIntegratedLoudness() const;
    void reset();

private:
    struct Biquad
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
        double z1 = 0.0, z2 = 0.0;
        double Q = 0.0, VH = 0.0, VB = 0.0, VL = 0.0, arctanK = 0.0;

        void init(double b0_48k, double b1_48k, double b2_48k, double a1_48k, double a2_48k);
        void prepare(double sampleRate);
        float process(float in);
        void resetState();
    };

    std::vector<Biquad> preFilters_;
    std::vector<Biquad> rlbcFilters_;

    double sampleRate_ = 48000.0;
    int numChannels_ = 2;

    int binSize_ = 0;
    int numBins_ = 4;
    int currentBin_ = 0;
    int samplesInCurrentBin_ = 0;
    std::vector<std::vector<double>> binSums_;

    int absoluteBlockCount_ = 0;
    double absoluteWeightedSum_ = 0.0;
    double relativeThreshold_ = -70.0;
    std::map<int, int> histogram_;
    float integratedLoudness_ = -300.0f;

    static constexpr float kMinValue = -300.0f;
    static constexpr double kAbsThreshold = -70.0;
    static constexpr double kLowestBlockToConsider = -100.0;

    static int iround(double d);
};
