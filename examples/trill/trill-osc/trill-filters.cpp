#include "trill-filters.h"
#include <cmath>

TrillBaseFilter::TrillBaseFilter(float pole, float thresh)
    : threshold_(thresh), last_value_(0.0f), baseline_(0.0f)
{
    filter_.setPole(pole);
    baseline_filter_.setPole(0.9995f);
}

float TrillBaseFilter::process(float input)
{
    // 1. Baseline tracking
    if (std::abs(input - baseline_) < 0.05f)
    {
        baseline_ = baseline_filter_.tick(input);
    }

    // 2. Subtract Baseline & Clamp
    float deBasalized = std::max(0.0f, input - baseline_);

    // 3. Noise Gate
    float cleanInput = (deBasalized < threshold_) ? 0.0f : deBasalized;

    // 4. STK Smoothing (OnePole)
    last_value_ = filter_.tick(cleanInput);
    return last_value_;
}

void TrillBaseFilter::recalibrate(float currentInput)
{
    baseline_ = currentInput;
    baseline_filter_.clear();
    baseline_filter_.tick(currentInput);
    filter_.clear();
    last_value_ = 0.0f;
}

float TrillBaseFilter::get_last_value() const { return last_value_; }
float TrillBaseFilter::get_baseline() const { return baseline_; }
void TrillBaseFilter::set_smoothing(float pole) { filter_.setPole(pole); }

TrillDeltaFilter::TrillDeltaFilter() {
    // Configure OneZero as a differentiator by setting the zero to 1.0
    // Equation: y[n] = x[n] - (1.0 * x[n-1])
    this->setB1(-1.0f);
}

float TrillDeltaFilter::process(float input) {
    return this->tick(input);
}
