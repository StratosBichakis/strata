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
#include <utility>
#include <random>
#include <sys/stat.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <map>

extern "C" {
#include <EXTERN.h>
#include <perl.h>
}
static PerlInterpreter *my_perl;

// Struct to hold bang details
struct BangDetails {
    // int bangs;
    // double tempo;
    // long last_tempo_change;
    double current_unix_time;
    std::string message;
};

class BangPerlClient  {
private:
    int sock;
    struct sockaddr_in server_addr;
    SV* perl_state_hv_ref; // Persistent Perl stat

public:
    BangPerlClient(const char* ip, int port) {
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

        // Initialize the persistent hash once
        dTHX;
        HV* hv = newHV();
        perl_state_hv_ref = newRV_noinc((SV*)hv);
        // We use newRV_noinc because we want this SV* to own the HV
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
        // std::cout << "Sent: " << message << std::endl;
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
        BangDetails details = {0};
        std::vector<std::string> my_scripts = {"bang.pl" };
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0, 1.0);

        try {
            while (true) {
                // Hot-reload check
                for (const auto& script : my_scripts) {
                    if (check_for_updates(script)) {
                        std::cout << "Hot-reloading: " << script << std::endl;
                        reload_perl_script(script.c_str());
                    }
                }
                
                // Create a Perl array (AV) of 32 random floats
                AV* message_av = newAV();
                for (int i = 0; i < 32; ++i) {
                    av_push(message_av, newSVnv(dis(gen)));
                }
                SV* message_sv = newRV_inc(reinterpret_cast<SV*>(message_av));

                details.current_unix_time = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();

                call_perl_tick(details, message_sv);
                
                if (!details.message.empty()) {
                    send_message(details.message);
                }

                std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>( 64 / 48000.0))); // Approximately 1.333ms
            }
        } catch (const std::exception& e) {
            std::cerr << "Error during message sending: " << e.what() << std::endl;
        }
    }
    
    ~BangPerlClient() {
        dTHX;
        if (perl_state_hv_ref) {
            SvREFCNT_dec(perl_state_hv_ref); // Correctly decrement reference count
        }
        close(sock);
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
        const char* ip = (argc > 1) ? argv[1] : "127.0.0.1";
        int port = (argc > 2) ? std::stoi(argv[2]) : 2001;
        
        BangPerlClient client(ip, port);
        client.connect_to_server();
        client.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    perl_destruct(my_perl);
    perl_free(my_perl);
    return 0;
}