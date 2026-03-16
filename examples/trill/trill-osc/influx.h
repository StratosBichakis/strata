#ifndef STRATA_INFLUX_H
#define STRATA_INFLUX_H

#include <OnePole.h>
#include "leaky-integrator.h"

class Influx {
private:
    int numInputs;
    int numOutputs;
    std::vector<std::vector<float>> weights;

    // Decoupled Leak: Each output has its own STK OnePole filter
    std::vector<LeakyIntegrator> integrators;
    std::vector<float> outputBuffer;
    float globalSensitivity;

public:
    Influx(int inputs, int outputs, unsigned int seed = 0);
    void process(const std::vector<float>& deltas);
    void setLeak(float leak);
    void setFrozen(bool freeze);
    void randomize(unsigned int seed);
    const std::vector<float>& getOutputs() const;
};

#endif //STRATA_INFLUX_H