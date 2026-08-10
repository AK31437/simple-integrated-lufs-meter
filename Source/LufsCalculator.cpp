#include "LufsCalculator.h"

void LufsCalculator::Biquad::init(double b0_48k, double b1_48k, double b2_48k,
                                   double a1_48k, double a2_48k)
{
    b0 = b0_48k; b1 = b1_48k; b2 = b2_48k;
    a1 = a1_48k; a2 = a2_48k;

    const double KoverQ = (2.0 - 2.0 * a2) / (a2 - a1 + 1.0);
    const double K = std::sqrt((a1 + a2 + 1.0) / (a2 - a1 + 1.0));
    Q = K / KoverQ;
    arctanK = std::atan(K);
    VB = (b0 - b2) / (1.0 - a2);
    VH = (b0 - b1 + b2) / (a2 - a1 + 1.0);
    VL = (b0 + b1 + b2) / (a1 + a2 + 1.0);
}

void LufsCalculator::Biquad::prepare(double sampleRate)
{
    if (sampleRate == 48000.0)
        return;

    const double K = std::tan(arctanK * 48000.0 / sampleRate);
    const double K2 = K * K;
    const double common = 1.0 / (1.0 + K / Q + K2);

    b0 = (VH + VB * K / Q + VL * K2) * common;
    b1 = 2.0 * (VL * K2 - VH) * common;
    b2 = (VH - VB * K / Q + VL * K2) * common;
    a1 = 2.0 * (K2 - 1.0) * common;
    a2 = (1.0 - K / Q + K2) * common;
}

float LufsCalculator::Biquad::process(float in)
{
    const double x = static_cast<double>(in);
    const double v = x - a1 * z1 - a2 * z2;
    const double out = b0 * v + b1 * z1 + b2 * z2;
    z2 = z1;
    z1 = v;
    return static_cast<float>(out);
}

void LufsCalculator::Biquad::resetState()
{
    z1 = 0.0;
    z2 = 0.0;
}

int LufsCalculator::iround(double d)
{
    return (d > 0.0) ? static_cast<int>(d + 0.5) : static_cast<int>(d - 0.5);
}

void LufsCalculator::prepare(double sampleRate, int numChannels)
{
    sampleRate_ = sampleRate;
    numChannels_ = numChannels;

    preFilters_.resize(static_cast<size_t>(numChannels));
    rlbcFilters_.resize(static_cast<size_t>(numChannels));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        preFilters_[static_cast<size_t>(ch)].init(
            1.53512485958697, -2.69169618940638, 1.19839281085285,
            -1.69065929318241, 0.73248077421585);
        preFilters_[static_cast<size_t>(ch)].prepare(sampleRate);

        rlbcFilters_[static_cast<size_t>(ch)].init(
            1.0, -2.0, 1.0,
            -1.99004745483398, 0.99007225036621);
        rlbcFilters_[static_cast<size_t>(ch)].prepare(sampleRate);
    }

    binSize_ = std::max(static_cast<int>(sampleRate * 0.1), 1);
    numBins_ = 4;
    currentBin_ = 0;
    samplesInCurrentBin_ = 0;

    binSums_.assign(static_cast<size_t>(numChannels), std::vector<double>(static_cast<size_t>(numBins_), 0.0));

    reset();
}

void LufsCalculator::reset()
{
    for (auto& f : preFilters_) f.resetState();
    for (auto& f : rlbcFilters_) f.resetState();

    for (auto& ch : binSums_)
        std::fill(ch.begin(), ch.end(), 0.0);

    currentBin_ = 0;
    samplesInCurrentBin_ = 0;
    absoluteBlockCount_ = 0;
    absoluteWeightedSum_ = 0.0;
    relativeThreshold_ = kAbsThreshold;
    histogram_.clear();
    integratedLoudness_ = kMinValue;
}

void LufsCalculator::processBlock(float* const* channelData, int numChannels, int numSamples)
{
    const int chCount = std::min(numChannels, numChannels_);
    if (chCount == 0 || binSize_ == 0)
        return;

    int pos = 0;

    while (pos < numSamples)
    {
        const int remaining = numSamples - pos;
        const int toBin = std::min(remaining, binSize_ - samplesInCurrentBin_);

        for (int ch = 0; ch < chCount; ++ch)
        {
            float* data = channelData[ch] + pos;
            double& binSum = binSums_[static_cast<size_t>(ch)][static_cast<size_t>(currentBin_)];

            for (int i = 0; i < toBin; ++i)
            {
                float filtered = preFilters_[static_cast<size_t>(ch)].process(data[i]);
                filtered = rlbcFilters_[static_cast<size_t>(ch)].process(filtered);
                binSum += static_cast<double>(filtered) * static_cast<double>(filtered);
            }
        }

        samplesInCurrentBin_ += toBin;
        pos += toBin;

        if (samplesInCurrentBin_ >= binSize_)
        {
            double weightedSum = 0.0;
            for (int ch = 0; ch < chCount; ++ch)
            {
                double meanSquare = 0.0;
                for (int b = 0; b < numBins_; ++b)
                    meanSquare += binSums_[static_cast<size_t>(ch)][static_cast<size_t>(b)];
                meanSquare /= static_cast<double>(numBins_ * binSize_);
                weightedSum += meanSquare;
            }

            if (weightedSum > 0.0)
            {
                const double blockLoudness = -0.691 + 10.0 * std::log10(weightedSum);

                if (blockLoudness > kAbsThreshold)
                {
                    ++absoluteBlockCount_;
                    absoluteWeightedSum_ += weightedSum;
                    relativeThreshold_ = -10.691 + 10.0 * std::log10(absoluteWeightedSum_ / absoluteBlockCount_);
                }

                if (blockLoudness > kLowestBlockToConsider)
                    histogram_[iround(blockLoudness * 10.0)] += 1;

                if (!histogram_.empty())
                {
                    const int thresholdKey = iround(relativeThreshold_ * 10.0) + 1;
                    auto it = histogram_.lower_bound(thresholdKey);

                    if (it != histogram_.end())
                    {
                        double sumForI = 0.0;
                        int countForI = 0;

                        for (; it != histogram_.end(); ++it)
                        {
                            countForI += it->second;
                            sumForI += static_cast<double>(it->second)
                                       * std::pow(10.0, (it->first * 0.1 + 0.691) * 0.1);
                        }

                        if (countForI > 0)
                            integratedLoudness_ = static_cast<float>(
                                -0.691 + 10.0 * std::log10(sumForI / countForI));
                        else
                            integratedLoudness_ = kMinValue;
                    }
                }
            }

            currentBin_ = (currentBin_ + 1) % numBins_;
            for (int ch = 0; ch < chCount; ++ch)
                binSums_[static_cast<size_t>(ch)][static_cast<size_t>(currentBin_)] = 0.0;
            samplesInCurrentBin_ = 0;
        }
    }
}

float LufsCalculator::getIntegratedLoudness() const
{
    return integratedLoudness_;
}
