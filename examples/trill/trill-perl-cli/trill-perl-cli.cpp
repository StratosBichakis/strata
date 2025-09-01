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

#include "libraries/Trill/Trill.h"

extern "C" {
#include <EXTERN.h>
#include <perl.h>
}
static PerlInterpreter *my_perl;

class TrillPerlClient  {
private:
    Trill touchSensor;
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

    std::string call_perl_skini_gen (const std::vector<float>& values) {
        dSP;                      // initialize stack pointer
        ENTER;                    // start new scope
        SAVETMPS;                 // save temporaries

        PUSHMARK(SP);             // prepare stack
        for (float v : values) {
            XPUSHs(sv_2mortal(newSVnv(v))); // push float as Perl number
        }
        PUTBACK;                  // make stack valid

        int count = call_pv("run", G_SCALAR);  // call perl function
        SPAGAIN;                  // refresh stack pointer

        std::string result;
        if (count == 1) {
            SV* sv = POPs;
            result = SvPV_nolen(sv);
        }
        PUTBACK;
        FREETMPS; LEAVE;

        return result;
    }

    void run() {
        // const std::string note_on = "NoteOn 0 1 60 114\r\n";
        // const std::string note_off = "NoteOff 0 1 60 114\r\n";
        bool is_note_on = true;

        if(touchSensor.setup(1, Trill::CRAFT) != 0) {
            fprintf(stderr, "Unable to initialise Trill Craft\n");
            // return false;
        }

        touchSensor.setPrescaler(2);

        try {
            while (true) {
                touchSensor.readI2C();
                std::vector<float> values = touchSensor.rawData;

                if (!values.empty()) { //
                    std::string msg = call_perl_skini_gen(values);
                    send_message(msg);
                }

                std::this_thread::sleep_for(std::chrono::microseconds(12000));
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
    const char *my_argv[] = { "", "skini-gen.pl", NULL };

    PERL_SYS_INIT3(&argc, &argv, &environ);
    my_perl = perl_alloc();
    perl_construct(my_perl);
    perl_parse(my_perl, NULL, 2, (char **) my_argv, (char **)NULL);
    perl_run(my_perl);
    // eval_pv("use lib '/root/Bela/projects';", TRUE);
    // eval_pv("require \"skini-gen.pl\";", TRUE);

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