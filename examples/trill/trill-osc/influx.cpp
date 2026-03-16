#include "influx.h"
#include <vector>
#include <random>

Influx::Influx(int inputs, int outputs, unsigned int seed)
    : num_inputs_(inputs), num_outputs_(outputs) {
    weights.resize(inputs, std::vector<float>(outputs, 0.0f));
    randomize(seed);
}

std::vector<float> Influx::process(const std::vector<float>& deltas) {
    std::vector<float> influences(num_outputs_, 0.0f);
    for (int out = 0; out < num_outputs_; ++out) {
        for (int in = 0; in < num_inputs_; ++in) {
            if (std::abs(deltas[in]) > 0.001f) {
                influences[out] += deltas[in] * weights[in][out];
            }
        }
    }
    return influences;
}

void Influx::randomize(unsigned int seed) {
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    for (auto& row : weights) {
        for (auto& w : row) w = distribution(generator);
    }
}