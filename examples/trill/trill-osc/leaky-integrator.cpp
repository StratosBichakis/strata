//
// Created by Stratos Bichakis on 16.03.26.
//

#include "leaky-integrator.h"

// Local clip helper for older libstdc++
float clip(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
}

LeakyIntegrator::LeakyIntegrator(float leak, float center)
    : leakCoeff(leak), centerPoint(center), isFrozen(false) {
    filter.setPole(0.0f);
    filter.tick(centerPoint);
    filter.setPole(leakCoeff);
}

void LeakyIntegrator::setLeak(float leak) {
    leakCoeff = clip(leak, 0.0f, 0.9999f);
    if (!isFrozen) filter.setPole(leakCoeff);
}

void LeakyIntegrator::setFrozen(bool freeze) {
    isFrozen = freeze;
    // Pole of 1.0 = no leak (infinite memory)
    filter.setPole(isFrozen ? 1.0f : leakCoeff);
}

float LeakyIntegrator::process(float influence) {
    // If frozen, drift target is current value (stays put)
    // If not frozen, drift target is centerPoint (0.5)
    float target = isFrozen ? filter.lastOut() : centerPoint;

    // Apply the push/influence to the current state
    float nextState = target + influence;

    // Tick through STK filter and clamp result
    return clip(filter.tick(nextState), -1.0f, 1.0f);
}

float LeakyIntegrator::getCurrentValue() const { return filter.lastOut(); }
