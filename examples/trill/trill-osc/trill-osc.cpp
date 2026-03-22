#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <stdexcept>

#include "libraries/Trill/Trill.h"
#include <oscpp/client.hpp>
#include <oscpp/server.hpp>

#include "trill-config.h" // Add this include
#include "trill-filters.h"
#include "influx.h"
#include "random"

class TrillClientUDP {
private:
    Trill touchSensor;
    int sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in plot_addr;
    struct sockaddr_in local_addr; // For receiving

    std::vector<TrillBaseFilter> base_filters_;
    std::vector<TrillDeltaFilter> delta_filters_;
    Influx influx_;
    std::vector<LeakyIntegrator> parameter_integrators_;
    // We'll use these to store the results before sending OSC
    std::vector<float> processed_values_;
    std::vector<float> delta_values_;

    std::vector<float> pin_weights_ = INPUT_WEIGHTS; // Store weights

    void handle_incoming_osc() {
        uint8_t rx_buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        // Non-blocking read
        ssize_t size = recvfrom(sock, rx_buffer, sizeof(rx_buffer), MSG_DONTWAIT,
                                (struct sockaddr*)&client_addr, &addr_len);

        if (size > 0) {
            try {
                OSCPP::Server::Packet packet(rx_buffer, size);
                if (packet.isMessage()) {
                    OSCPP::Server::Message msg(packet);
                    std::string addr = msg.address();

                    if (addr == "/recalibrate") {
                        trigger_recalibrate();
                    }
                    else if (addr == "/set_smoothing" ) {
                        // Example: Change smoothing pole for all sensors
                        auto args = msg.args();
                        if (!args.atEnd()) {
                            float newPole = args.next<float>();

                            for(auto &p : base_filters_) {
                                p.set_smoothing(newPole);
                            }
                            std::cout << "Smoothing updated to: " << newPole << std::endl;
                        }
                    }
                    else if (addr == "/randomize_matrix") {
                        auto args = msg.args();
                        if (!args.atEnd()) {
                            // Randomize with a specific seed provided by SuperCollider
                            unsigned int newSeed = (unsigned int)args.next<int32_t>();
                            influx_.randomize(newSeed);
                            std::cout << "Matrix re-seeded with: " << newSeed << std::endl;
                        } else {
                            std::random_device rd;
                            unsigned int randomSeed = rd();
                            influx_.randomize(randomSeed);
                        }
                    }
                    else if (addr == "/set_leak") {
                        auto args = msg.args();
                        if (!args.atEnd()) {
                            float leakValue = args.next<float>();
                            for(auto &li : parameter_integrators_) {
                                li.setLeak(leakValue);
                            }
                            std::cout << "Leak Coefficient updated: " << leakValue << std::endl;
                        }
                    }
                    else if (addr == "/freeze") {
                        auto args = msg.args();
                        if (!args.atEnd()) {
                            bool freezeState = (args.next<int32_t>() > 0);
                            for(auto &li : parameter_integrators_) {
                                li.setFrozen(freezeState);
                            }
                        }
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "OSC Parse Error: " << e.what() << std::endl;
            }
        }
    }

    void trigger_recalibrate() {
        touchSensor.readI2C();
        for (int i = 0; i < NUM_ACTIVE_INPUTS; ++i) {
            int pinIdx = ACTIVE_PINS[i];
            base_filters_[i].recalibrate(touchSensor.rawData[pinIdx]);
        }
        std::cout << "Baseline recalibrated for active pins." << std::endl;
    }

public:
    TrillClientUDP(const char* ip, int port) :
    processed_values_(NUM_ACTIVE_INPUTS, 0.0f),
    delta_values_(NUM_ACTIVE_INPUTS, 0.0f),
    influx_(NUM_ACTIVE_INPUTS, NUM_OUTPUT_CHANNELS, 1234) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
            throw std::runtime_error("Failed to create UDP socket");

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
            throw std::runtime_error("Invalid IP address");

        memset(&plot_addr, 0, sizeof(plot_addr));
        plot_addr.sin_family = AF_INET;
        plot_addr.sin_port = htons(57120); // SuperCollider
        inet_pton(AF_INET, ip, &plot_addr.sin_addr);

        // 3. Setup Local Address (Where we listen for /recalibrate)
        // We listen on the same port we send to, or a different one (e.g., 57121)
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY;
        local_addr.sin_port = htons(7562); // Choose a port for incoming commands

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }

        // Initialize processors only for active pins
        base_filters_.reserve(NUM_ACTIVE_INPUTS);
        delta_filters_.reserve(NUM_ACTIVE_INPUTS);

        for(int i = 0; i < NUM_ACTIVE_INPUTS; ++i) {
            base_filters_.emplace_back(0.3f, 0.01f);
            delta_filters_.emplace_back(); // Initialize the differentiators
        }

        parameter_integrators_.reserve(NUM_OUTPUT_CHANNELS);
        for(int i = 0; i < NUM_OUTPUT_CHANNELS; ++i) {
            parameter_integrators_.emplace_back(0.05f, 0.0f);
        }

    }

    void send_osc_raw_values(const std::string& address, const std::vector<float>& values) {
        uint8_t buffer[1024];  // Bigger buffer to hold all sensor values
        // std::array<char,kMaxPacketSize> buffer;
        OSCPP::Client::Packet packet(buffer, sizeof(buffer));
        // packet.openMessage(address.c_str(), values.size());
        packet.openMessage(address.c_str(), values.size());
        // packet.openArray();

        for (auto val: values) {
            packet.float32(val);
        }
        // packet.closeArray();

        packet.closeMessage();
        size_t len = packet.size();

        if (sendto(sock, buffer, len, 0,
                   (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            throw std::runtime_error("Failed to send OSC message");
        }
        if (sendto(sock, buffer, len, 0,
           (struct sockaddr*)&plot_addr, sizeof(plot_addr)) < 0) {
            throw std::runtime_error("Failed to send OSC message");
           }

        // std::cout << "Sent OSC with " << count << " values." << std::endl;
    }

    void run() {
        if (touchSensor.setup(1, Trill::CRAFT) != 0) {
            std::cerr << "Unable to initialise Trill Craft\n";
            return;
        }
        touchSensor.setPrescaler(2);
        touchSensor.setMode(Trill::RAW);

        bool initialCalibrationDone = false;
        int warmupFrames = 0; // New counter

        // const auto OSC_ADDRESS = std::string("/trill/raw");

        try {
            while (true) {
                // 1. Check for external reset commands
                handle_incoming_osc();

                if(touchSensor.readI2C() == 0) {
                    if (!initialCalibrationDone) {
                        if (warmupFrames < 20) {
                            warmupFrames++;
                        } else {
                            int successCount = 0;
                            for (int i = 0; i < NUM_ACTIVE_INPUTS; ++i) {
                                int pinIdx = ACTIVE_PINS[i];
                                // Attempt recalibrate; if any pin is > 0.8, it returns false
                                if (base_filters_[i].recalibrate(touchSensor.rawData[pinIdx])) {
                                    successCount++;
                                }
                            }

                            if (successCount >= (NUM_ACTIVE_INPUTS - 2)) {
                                initialCalibrationDone = true;
                                std::cout << "Initial baseline synced successfully." << std::endl;
                            } else {
                                // Optional: Print a warning that the sensor is blocked
                                std::cout << "Calibration deferred: Sensor active." << std::endl;
                            }
                        }
                    }

                    // 1. Spatial Filtering (Input side)
                    for (int i = 0; i < NUM_ACTIVE_INPUTS; ++i) {
                        int pinIdx = ACTIVE_PINS[i];
                        float raw_filtered = base_filters_[i].process(touchSensor.rawData[pinIdx]);
                        processed_values_[i] = std::min(1.0f, raw_filtered * pin_weights_[i]);
                        delta_values_[i] = delta_filters_[i].process(processed_values_[i]);
                    }

                    // 2. Mapping Stage
                    // This returns a vector of 8 "influences"
                    std::vector<float> influences = influx_.process(delta_values_);

                    // 3. Temporal Integration Stage (Output side)
                    std::vector<float> integrated_outputs(8);
                    float global_sensitivity = 5.0f; // Adjust as needed

                    for (int i = 0; i < NUM_OUTPUT_CHANNELS; ++i) {
                        // Each integrator tracks the state of one parameter
                        integrated_outputs[i] = parameter_integrators_[i].process(influences[i] * global_sensitivity);
                    }

                    // 4. Send the integrated parameter values
                    if ( initialCalibrationDone )
                    {
                        send_osc_raw_values("/trill/raw", processed_values_);
                        send_osc_raw_values("/trill/delta", delta_values_);
                        send_osc_raw_values("/trill/influx", integrated_outputs);
                    }

                }
                std::this_thread::sleep_for(std::chrono::milliseconds(11));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    ~TrillClientUDP() {
        close(sock);
    }
};

int main(int argc, char* argv[]) {
    try {
        const char* ip = (argc > 1 ? argv[1] : "127.0.0.1");
        int port = (argc > 2 ? std::stoi(argv[2]) : 2002);

        TrillClientUDP client(ip, port);
        client.run();

    } catch (const std::exception& e) {
        std::cerr << "Startup error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}