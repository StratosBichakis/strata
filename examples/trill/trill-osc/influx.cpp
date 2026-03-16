#include "influx.h"

#include <vector>
#include <random>
#include <algorithm>
#include <OnePole.h> // Reuse STK components


Influx::Influx(int inputs, int outputs, unsigned int seed)
    : numInputs(inputs), numOutputs(outputs), globalSensitivity(2.5f) {

    weights.resize(inputs, std::vector<float>(outputs, 0.0f));
    integrators.resize(outputs);
    outputBuffer.resize(outputs);

    // Initialize an integrator for every output parameter
    for(int i = 0; i < outputs; ++i) {
        integrators.emplace_back(0.995f, 0.5f);
    }

    randomize(seed);
}

void Influx::setLeak(float leak) {
    for(auto& i : integrators) i.setLeak(leak);
}

void Influx::process(const std::vector<float>& deltas) {
    for (int out = 0; out < numOutputs; ++out) {
        float totalInfluence = 0.0f;
        for (int in = 0; in < numInputs; ++in) {
            if (std::abs(deltas[in]) > 0.001f) {
                totalInfluence += deltas[in] * weights[in][out];
            }
        }
        // Pass the summed influence to the specific integrator
        outputBuffer[out] = integrators[out].process(totalInfluence * globalSensitivity);
    }
}

void Influx::setFrozen(bool freeze) {
    for(auto& i : integrators) i.setFrozen(freeze);
}

void Influx::randomize(unsigned int seed) {
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    for (auto& row : weights) {
        for (auto& w : row) w = distribution(generator);
    }
}

const std::vector<float>& Influx::getOutputs() const
{
    return outputBuffer;
}
