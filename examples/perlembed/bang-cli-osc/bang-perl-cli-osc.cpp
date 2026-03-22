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
#include <vector>
#include <sys/stat.h>
#include <limits.h>
#include <map>

extern "C" {
#include <EXTERN.h>
#include <perl.h>
}

#include <oscpp/server.hpp>

static PerlInterpreter *my_perl;

// Struct to hold bang details
struct BangDetails {
    double current_unix_time;
    std::string message;
};

class BangPerlOSCClient  {
private:
    int osc_sock;
    int tcp_sock;
    struct sockaddr_in osc_local_addr;
    struct sockaddr_in tcp_server_addr;
    SV* perl_state_hv_ref; // Persistent Perl stat
    std::vector<float> latest_osc_data;

public:
    BangPerlOSCClient(const char* ip, int tcp_port, int osc_port) : latest_osc_data(8, 0.0f) {
        osc_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (osc_sock < 0) throw std::runtime_error("Failed to create UDP socket");

        int reuse = 1;
        if (setsockopt(osc_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
            perror("setsockopt(SO_REUSEADDR) failed");
        }
#ifdef SO_REUSEPORT
        if (setsockopt(osc_sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) {
            perror("setsockopt(SO_REUSEPORT) failed");
        }
#endif

        // Set to non-blocking so we can "drain" the queue without stopping
        int flags = fcntl(osc_sock, F_GETFL, 0);
        fcntl(osc_sock, F_SETFL, flags | O_NONBLOCK);

        memset(&osc_local_addr, 0, sizeof(osc_local_addr));
        osc_local_addr.sin_family = AF_INET;
        osc_local_addr.sin_addr.s_addr = INADDR_ANY;
        osc_local_addr.sin_port = htons(osc_port);

        if (bind(osc_sock, (struct sockaddr *)&osc_local_addr, sizeof(osc_local_addr)) < 0) {
            throw std::runtime_error("Failed to bind OSC socket");
        }

        tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (tcp_sock < 0) {
            throw std::runtime_error("Failed to create TCP socket");
        }

        // Setup server address structure
        memset(&tcp_server_addr, 0, sizeof(tcp_server_addr));
        tcp_server_addr.sin_family = AF_INET;
        tcp_server_addr.sin_port = htons(tcp_port);

        if (inet_pton(AF_INET, ip, &tcp_server_addr.sin_addr) <= 0) {
            close(tcp_sock);
            throw std::runtime_error("Invalid address/Address not supported");
        }

        // Initialize the persistent hash once
        dTHX;
        HV* hv = newHV();
        perl_state_hv_ref = newRV_noinc((SV*)hv);
    }

    void connect_tcp() {
        if (connect(tcp_sock, (struct sockaddr*)&tcp_server_addr, sizeof(tcp_server_addr)) < 0) {
            close(tcp_sock);
            throw std::runtime_error("Connection failed");
        }
        std::cout << "Connected to server successfully" << std::endl;
    }

    bool check_for_updates(const std::string& filename) {
        static std::map<std::string, time_t> last_mtimes;
        struct stat result;

        // 1. Resolve the physical path (follows symlinks to the source folder)
        char actualpath[PATH_MAX + 1];
        char *ptr = realpath(filename.c_str(), actualpath);

        // If realpath fails (file missing), use the raw filename as fallback
        const char* path_to_check = ptr ? actualpath : filename.c_str();

        if (stat(path_to_check, &result) == 0) {
            // If this is the first time we see this file, just store the time
            if (last_mtimes.find(filename) == last_mtimes.end()) {
                last_mtimes[filename] = result.st_mtime;
                return false;
            }

            if (result.st_mtime > last_mtimes[filename]) {
                last_mtimes[filename] = result.st_mtime;
                return true;
            }
        }
        return false;
    }

    void reload_perl_script(const char* filename) {
        dTHX;

        // Check syntax before committing: perl -c filename
        std::string check_cmd = "perl -c " + std::string(filename) + " 2>&1";
        FILE* pipe = popen(check_cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string result = "";
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) result += buffer;
            int status = pclose(pipe);

            if (status != 0) {
                std::cerr << "SYNTAX ERROR in Perl script. Reload aborted.\n" << result << std::endl;
                return; // Exit without reloading the broken code
            }
        }

        // If syntax is OK, proceed with 'do'
        std::string command = "do '" + std::string(filename) + "';";
        eval_pv(command.c_str(), TRUE);

        if (SvTRUE(ERRSV)) {
            std::cerr << "RUNTIME ERROR during reload: " << SvPV_nolen(ERRSV) << std::endl;
        } else {
            std::cout << "Script " << filename << " hot-reloaded successfully." << std::endl;
        }
    }

    void call_perl_tick(BangDetails& details, SV* message_sv) {
        dTHX;
        dSP;
        ENTER;
        SAVETMPS;

        // Sync C++ struct values TO the persistent Perl hash before calling
        HV* hv = (HV*)SvRV(perl_state_hv_ref);
        hv_stores(hv, "current_unix_time", newSVnv(details.current_unix_time));

        PUSHMARK(SP);
        XPUSHs(perl_state_hv_ref); // Pass the persistent hash
        XPUSHs(sv_2mortal(message_sv));
        PUTBACK;

        call_pv("tick", G_DISCARD);

        SPAGAIN;
        // After call, pull ONLY what C++ needs back into the struct
        SV** sv_tempo = hv_fetchs(hv, "tempo", 0);
        // if (sv_tempo) details.tempo = SvNV(*sv_tempo);
        // std::cout << "C++ DEBUG - Tempo : " << details.tempo << std::endl;
        SV** sv_msg = hv_fetchs(hv, "message", 0);
        if (sv_msg) details.message = SvPV_nolen(*sv_msg);

        PUTBACK;
        FREETMPS;
        LEAVE;
    }


    void run() {
        uint8_t buffer[2048];
        BangDetails details;
        std::vector<std::string> my_scripts = {"bang.pl" };

        std::cout << "Listening for OSC on port " << ntohs(osc_local_addr.sin_port) << "..." << std::endl;


        // try {
            while (true) {
                // Hot-reload check
                for (const auto& script : my_scripts) {
                    if (check_for_updates(script)) {
                        std::cout << "Hot-reloading: " << script << std::endl;
                        reload_perl_script(script.c_str());
                    }
                }

                while (true) {
                    ssize_t size = recvfrom(osc_sock, buffer, sizeof(buffer), 0, NULL, NULL);
                    if (size < 0) break; // Queue is empty

                    try {
                        OSCPP::Server::Packet packet(buffer, size);
                        if (packet.isMessage()) {
                            OSCPP::Server::Message msg(packet);

                            auto args = msg.args();

                            // Check for the specific address sent by trill-osc.cpp
                            if (std::string(msg.address()) == "/trill/influx") {
                                auto args = msg.args();
                                std::vector<float> temp_data;

                                while (!args.atEnd()) {
                                    temp_data.push_back(args.next<float>());
                                }

                                // Only update if we actually received data
                                if (!temp_data.empty()) {
                                    latest_osc_data = temp_data;
                                }
                            }
                        }
                    } catch (const std::exception& e) {
                        // std::cerr << "Error receiving message: " << e.what() << std::endl;
                        }
                }

                dTHX;
                AV* message_av = newAV();
                for (float val : latest_osc_data) {
                    av_push(message_av, newSVnv(val));
                }
                SV* message_sv = newRV_inc(reinterpret_cast<SV*>(message_av));

                details.current_unix_time = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();

                call_perl_tick(details, message_sv);

                if (!details.message.empty()) {
                    send(tcp_sock, details.message.c_str(), details.message.length(), 0);
                }

                std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>( 64 / 48000.0))); // Approximately 1.333ms
                // std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>( 4800 / 48000.0))); // Approximately 1.333ms
            }
        // } catch (const std::exception& e) {
            // std::cerr << "Error during message sending: " << e.what() << std::endl;
        // }
    }
    
    ~BangPerlOSCClient() {
        dTHX;
        if (perl_state_hv_ref) {
            SvREFCNT_dec(perl_state_hv_ref); // Correctly decrement reference count
        }
        close(osc_sock);
        close(tcp_sock);
    }
};

int main(int argc, char* argv[]) {
    const char *my_argv[] = { "", "bang.pl", NULL };

    PERL_SYS_INIT3(&argc, &argv, &environ);
    my_perl = perl_alloc();
    perl_construct(my_perl);
    perl_parse(my_perl, NULL, 2, (char **) my_argv, (char **)NULL);
    perl_run(my_perl);

    try {
        // Usage: ./prog [TCP_IP] [TCP_PORT] [OSC_LISTEN_PORT]
        const char* ip = (argc > 1) ? argv[1] : "127.0.0.1";
        int port = (argc > 2) ? std::stoi(argv[2]) : 2001;

        int tcp_port = (argc > 2) ? std::stoi(argv[2]) : 2001;
        int osc_port = (argc > 3) ? std::stoi(argv[3]) : 2002;

        BangPerlOSCClient client(ip, tcp_port, osc_port);
        client.connect_tcp();
        client.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    perl_destruct(my_perl);
    perl_free(my_perl);
    PERL_SYS_TERM();
    return 0;
}