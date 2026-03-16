//
// Created by Stratos Bichakis on 15.03.26.
//

#ifndef STRATA_TRILL_PROCESSOR_H
#define STRATA_TRILL_PROCESSOR_H

#include <OnePole.h>
#include <OneZero.h>

class TrillBaseFilter {
public:
    TrillBaseFilter(float pole = 0.9f, float thresh = 0.02f);

    float update(float input);

    float getDelta() const;
    float getLastValue() const;
    float getBaseline() const;
    void setSmoothing(float pole);
    void recalibrate(float currentInput);

private:
    stk::OnePole filter;
    stk::OnePole baselineFilter;  // Ultra-slow filter for drift tracking
    stk::OneZero deltaFilter;   // Velocity/Influence (High-pass/Differentiator)

    float threshold;
    float lastValue;
    float baseline;
};


class TrillDeltaFilter : public stk::OnePole
{
    public: TrillDeltaFilter();

};

#endif