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

#include "network-helper.hpp"
#include "algorithms/loop-network.hpp"
#include "algorithms/single-message.hpp"
#include "algorithms/byzantine-reliable-broadcast.hpp"

#include "logger.hpp"
#include "utils.hpp"

void signal_handler(int signum);

Logger* logger_ptr = nullptr;
int em_id;

int main(int argc, char* argv[]) {
    em_id = std::stoi(std::string(argv[1]));
    logger_ptr = new Logger("logging_dummy_" + std::to_string(em_id) + ".txt");
    signal(SIGINT, signal_handler);
    
    NetworkHelper net = NetworkHelper(em_id, std::string(argv[2]));
    logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, "Start of %d", em_id);

    // LoopNetwork ln = LoopNetwork(em_id, net);
    // ln.start("Hello!", 1);
    SingleMessage sm = SingleMessage(em_id, net);
    sm.start("Hello man!");

    while(true) {}
    return 0;
}

void signal_handler(int signum) {
    delete logger_ptr;

    char buf[BUF_SIZE];
    size_t offset = 0;

    offset += Logger::push_to_buffer_string_safe(buf + offset, "[DUMMY ");
    offset += Logger::push_to_buffer_int_safe(buf + offset, em_id);
    offset += Logger::push_to_buffer_string_safe(buf + offset, "]");

    switch (signum) {
    case SIGINT:
        offset += Logger::push_to_buffer_string_safe(buf + offset, "[SIGINT]");
        break;
    case SIGSEGV:
        offset += Logger::push_to_buffer_string_safe(buf + offset, "[SIGSEGV]");
        break;
    default:
        offset += Logger::push_to_buffer_string_safe(buf + offset, "[SIGINT]");
        break;
    }

    offset += Logger::push_to_buffer_time_safe(buf + offset, CLOCK_PROCESS_CPUTIME_ID);
    offset += Logger::push_to_buffer_string_safe(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);

    _exit(signum);
}
