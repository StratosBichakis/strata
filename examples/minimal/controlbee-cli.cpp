/*controlbee-cli.cpp*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <string>
#include <stdexcept>

class TrillPerlClient  {
private:
    int sock;
    struct sockaddr_in server_addr;
    
public:
    TrillPerlClient(const char* ip, int port) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        // Setup server address structure
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
            close(sock);
            throw std::runtime_error("Invalid address/Address not supported");
        }
    }
    
    void connect_to_server() {
        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            throw std::runtime_error("Connection failed");
        }
        std::cout << "Connected to server successfully" << std::endl;
    }
    
    void send_message(const std::string& message) {
        if (send(sock, message.c_str(), message.length(), 0) < 0) {
            throw std::runtime_error("Failed to send message");
        }
        std::cout << "Sent: " << message;
    }
    
    void run() {
        const std::string note_on = "NoteOn 0 1 60 114\r\n";
        const std::string note_off = "NoteOff 0 1 60 114\r\n";
        bool is_note_on = true;
        
        try {
            while (true) {
                send_message(is_note_on ? note_on : note_off);
                is_note_on = !is_note_on;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error during message sending: " << e.what() << std::endl;
        }
    }
    
    ~TrillPerlClient() {
        close(sock);
    }
};

int main(int argc, char* argv[]) {
    try {
        // Default to localhost:8080 if no arguments provided
        const char* ip = (argc > 1) ? argv[1] : "127.0.0.1";
        int port = (argc > 2) ? std::stoi(argv[2]) : 2001;
        
        TrillPerlClient client(ip, port);
        client.connect_to_server();
        client.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}