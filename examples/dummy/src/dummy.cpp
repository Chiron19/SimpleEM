#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <cstdlib>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

#include "dummy.hpp"
#include "algorithms.hpp"

#include "utils.hpp"

FILE *logging_fptr;

int main(int argc, char* argv[]) {
    logging_fptr = open_logging("dummy", true);
    signal(SIGINT, sigint_handler);
    
    int em_id = std::stoi(std::string(argv[1]));
    NetworkInterface net = NetworkInterface(em_id, std::string(argv[2]));
    std::string message;

    log_event_proc_cpu_time("Start of %d", getpid());

    for (int i = 0;; ++i) {
        for (int ii = 0; ii < 1e5; ++ii) {}

        message = net.receive();
        if (!message.empty()) {
            log_event_proc_cpu_time("Received message: %s", message.c_str());
        }

        if (i >= 3)
            continue;


        message = "[PING from " + std::to_string(em_id) + "]";
        net.send((i + 1) % net.addresses.size(), message);

        log_event_proc_cpu_time("Sent message");
    }

    return 0;
}

void sigint_handler(int signum) {
    fclose(logging_fptr);

    char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf + offset, "[DUMMY][SIGINT]");
    offset += push_to_buffer_time(buf + offset, CLOCK_PROCESS_CPUTIME_ID);
    offset += push_to_buffer_string(buf + offset, "[");
    offset += push_to_buffer_int(buf + offset, getpid());
    offset += push_to_buffer_string(buf + offset, "]");
    offset += push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);

    _exit(signum);
}
