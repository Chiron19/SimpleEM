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

    PingPong pp(em_id, net);
    pp.start(std::to_string(getpid()), 5);

    // Broadcast broadcast(em_id, net);
    // broadcast.start(std::to_string(getpid()));

    // SingleMessage sm(em_id, net);
    // sm.start(std::to_string(getpid()));


    while(true) {}
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
