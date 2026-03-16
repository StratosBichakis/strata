#include "trill-filters.h"
#include <cmath>

TrillBaseFilter::TrillBaseFilter(float pole, float thresh)
    : threshold(thresh), lastValue(0.0f), baseline(0.0f)
{
    filter.setPole(pole);
    baselineFilter.setPole(0.9995f);

    // Configure OneZero as a differentiator: y[n] = x[n] - x[n-1]
    // Setting the zero to 1.0 creates a High-Pass response.
    deltaFilter.setZero(1.0f);
}

float TrillBaseFilter::update(float input)
{
    // 1. Baseline tracking
    if (std::abs(input - baseline) < 0.05f)
    {
        baseline = baselineFilter.tick(input);
    }

    // 2. Subtract Baseline & Clamp
    float deBasalized = std::max(0.0f, input - baseline);

    // 3. Noise Gate
    float cleanInput = (deBasalized < threshold) ? 0.0f : deBasalized;

    // 4. STK Smoothing (OnePole)
    lastValue = filter.tick(cleanInput);

    // 5. STK Delta/Velocity (OneZero)
    // We tick the smoothed value through the OneZero to get the 'Influence'
    deltaFilter.tick(lastValue);

    return lastValue;
}

float TrillBaseFilter::getDelta() const
{
    // Returns the result of the last differentiator calculation
    return deltaFilter.lastOut();
}

float TrillBaseFilter::getLastValue() const
{
    return lastValue;
}

float TrillBaseFilter::getBaseline() const
{
    return baseline;
}

void TrillBaseFilter::setSmoothing(float pole)
{
    filter.setPole(pole);
}

void TrillBaseFilter::recalibrate(float currentInput)
{
    baseline = currentInput;
    baselineFilter.clear();
    baselineFilter.tick(currentInput);

    filter.clear();
    deltaFilter.clear();
    lastValue = 0.0f;
}
