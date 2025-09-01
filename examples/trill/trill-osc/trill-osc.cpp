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
#include <oscpp/client.hpp>  // Correct header for oscpp

class TrillClientUDP {
private:
    Trill touchSensor;
    int sock;
    struct sockaddr_in server_addr;

public:
    TrillClientUDP(const char* ip, int port) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
            throw std::runtime_error("Failed to create UDP socket");

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
            throw std::runtime_error("Invalid IP address");
    }

    void send_osc_raw_values(const std::string& address, const std::vector<float>& values) {
        uint8_t buffer[1024];  // Bigger buffer to hold all sensor values
        // std::array<char,kMaxPacketSize> buffer;
        OSCPP::Client::Packet packet(buffer, sizeof(buffer));
        // packet.openMessage(address.c_str(), values.size());
        packet.openMessage(address.c_str(), OSCPP::Tags::array(values.size()));
        packet.openArray();

        for (auto val: values) {
            packet.float32(val);
        }
        packet.closeArray();

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
                touchSensor.readI2C();
                send_osc_raw_values(OSC_ADDRESS, touchSensor.rawData);

                std::this_thread::sleep_for(std::chrono::microseconds(12000));
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