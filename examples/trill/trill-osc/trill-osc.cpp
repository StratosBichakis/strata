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
#include "trill-filters.h"
#include "influx.h"
#include "random"

class TrillClientUDP {
private:
    Trill touchSensor;
    int sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in local_addr; // For receiving

    std::vector<TrillBaseFilter> processors;
    Influx influx;
    // We'll use these to store the results before sending OSC
    std::vector<float> processedValues;
    std::vector<float> deltaValues;
    std::vector<float> accValues;


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

                            for(auto &p : processors) {
                                p.setSmoothing(newPole);
                            }
                            std::cout << "Smoothing updated to: " << newPole << std::endl;
                        }
                    }
                    else if (addr == "/randomize_matrix") {
                        auto args = msg.args();
                        if (!args.atEnd()) {
                            // Randomize with a specific seed provided by SuperCollider
                            unsigned int newSeed = (unsigned int)args.next<int32_t>();
                            influx.randomize(newSeed);
                            std::cout << "Matrix re-seeded with: " << newSeed << std::endl;
                        } else {
                            std::random_device rd;
                            unsigned int randomSeed = rd();
                            influx.randomize(randomSeed);
                        }
                    }
                    else if (addr == "/set_leak") {
                        auto args = msg.args();
                        if (!args.atEnd()) {
                            float leakValue = args.next<float>(); // e.g., 0.99 for slow, 0.8 for fast
                            influx.setLeak(leakValue);
                            std::cout << "Leak Coefficient updated: " << leakValue << std::endl;
                        }
                    }
                    else if (addr == "/freeze") {
                        auto args = msg.args();
                        if (!args.atEnd()) {
                            // OSC booleans are often sent as ints (0 or 1)
                            bool freezeState = (args.next<int32_t>() > 0);
                            influx.setFrozen(freezeState);
                            std::cout << "Freeze mode: " << (freezeState ? "ON" : "OFF") << std::endl;
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
        for (size_t i = 0; i < touchSensor.rawData.size(); ++i) {
            processors[i].recalibrate(touchSensor.rawData[i]);
            accValues[i] = 0.0f;
        }
        std::cout << "Baseline recalibrated for all 30 pins." << std::endl;
    }

public:
    TrillClientUDP(const char* ip, int port) : processedValues(30, 0.0f), deltaValues(30, 0.0f), accValues(30, 0.0f), influx(30, 8, 1234) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
            throw std::runtime_error("Failed to create UDP socket");

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
            throw std::runtime_error("Invalid IP address");

        // 3. Setup Local Address (Where we listen for /recalibrate)
        // We listen on the same port we send to, or a different one (e.g., 57121)
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY;
        local_addr.sin_port = htons(7562); // Choose a port for incoming commands

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }

        // Initialize 30 processors (for Trill Craft)
        processedValues.resize(30);
        deltaValues.resize(30);
        for(int i = 0; i < 30; ++i) {
            processors.emplace_back(0.3f, 0.02f);
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

        // std::cout << "Sent OSC with " << count << " values." << std::endl;
    }

    void run() {
        if (touchSensor.setup(1, Trill::CRAFT) != 0) {
            std::cerr << "Unable to initialise Trill Craft\n";
            return;
        }
        touchSensor.setPrescaler(2);
        touchSensor.setMode(Trill::RAW);

        const auto OSC_ADDRESS = std::string("/trill/raw");

        try {
            while (true) {
                // 1. Check for external reset commands
                handle_incoming_osc();

                if(touchSensor.readI2C() == 0) {
                    for (size_t i = 0; i < touchSensor.rawData.size(); ++i) {
                        processedValues[i] = processors[i].update(touchSensor.rawData[i]);
                        deltaValues[i] = processors[i].getDelta();

                        // float sensitivity = 5.0f;
                        // accValues[i] += deltaValues[i] * sensitivity;
                    }

                    // Influx Mapping
                    influx.process(deltaValues); //

                    send_osc_raw_values("/trill/raw", processedValues);
                    // send_osc_raw_values("/trill/delta", deltaValues);
                    send_osc_raw_values("/trill/influx", influx.getOutputs());
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
        int port = (argc > 2 ? std::stoi(argv[2]) : 57120);

        TrillClientUDP client(ip, port);
        client.run();

    } catch (const std::exception& e) {
        std::cerr << "Startup error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}