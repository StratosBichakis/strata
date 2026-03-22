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

    float process(float input);
    float get_last_value() const;
    float get_baseline() const;
    void set_smoothing(float pole);
    bool recalibrate(float currentInput);

private:
    stk::OnePole filter_;
    stk::OnePole baseline_filter_;  // Ultra-slow filter for drift tracking

    float threshold_;
    float last_value_;
    float baseline_;
};


class TrillDeltaFilter : public stk::OneZero
{
    public: TrillDeltaFilter();
    float process(float input);
};

#endif