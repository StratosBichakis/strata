//
// Created by Stratos Bichakis on 17.03.26.
//

#ifndef STRATA_TRILL_CONFIG_H
#define STRATA_TRILL_CONFIG_H

#include <vector>

// Define which Trill Craft pins are physically connected
const std::vector<int> ACTIVE_PINS = {
    0, 1, 2, 3,       // First cluster
    4, 5, 6, 7,       // Second cluster
    8, 9, 10, 11,     // Third cluster
    20, 21, 22    // Fourth cluster
};

const std::vector<float> INPUT_WEIGHTS = {
    1.0f, 1.0f, 1.2f, 1.0f, // Cluster 1 (e.g., pin 2 is a bit weak)
    1.0f, 1.5f, 1.0f, 1.0f, // Cluster 2
    1.5f, 1.5f, 1.5f, 1.5f, // Cluster 3 (e.g., this whole cluster has a weaker signal)
    2.0f, 2.0f, 2.0 // Cluster 4
};

// Derived constants
const int NUM_ACTIVE_INPUTS = ACTIVE_PINS.size(); // Total: 16
const int NUM_OUTPUT_CHANNELS = 8;

#endif //STRATA_TRILL_CONFIG_H