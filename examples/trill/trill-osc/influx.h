#ifndef STRATA_INFLUX_H
#define STRATA_INFLUX_H

#include <OnePole.h>
#include "leaky-integrator.h"

class Influx {
private:
    int num_inputs_;
    int num_outputs_;
    std::vector<std::vector<float>> weights;
public:
    Influx(int inputs, int outputs, unsigned int seed = 0);
    std::vector<float> process(const std::vector<float>& deltas);
    void randomize(unsigned int seed);
};

#endif //STRATA_INFLUX_H