//
// Created by Stratos Bichakis on 16.03.26.
//

#ifndef STRATA_LEAKY_INTEGRATOR_H
#define STRATA_LEAKY_INTEGRATOR_H

#include <OnePole.h>

class LeakyIntegrator {
private:
    stk::OnePole filter;
    float leakCoeff;
    float centerPoint;
    bool isFrozen;
public:
    LeakyIntegrator(float leak = 0.995f, float center = 0.5f);
    void setLeak(float leak);
    void setFrozen(bool freeze);
    float process(float influence);
    float getCurrentValue() const;
};



#endif //STRATA_LEAKY_INTEGRATOR_H