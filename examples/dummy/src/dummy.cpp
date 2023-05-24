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

Logger* logger_ptr = nullptr;

int main(int argc, char* argv[]) {
    int em_id = std::stoi(std::string(argv[1]));
    logger_ptr = new Logger("logging_dummy_" + std::to_string(em_id) + ".txt");
    signal(SIGINT, sigint_handler);
    
    NetworkHelper net = NetworkHelper(em_id, std::string(argv[2]));
    logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, "Start of %d", getpid());

    // LoopNetwork ln = LoopNetwork(em_id, net);
    // ln.start("Hello!", 1);
    SingleMessage sm = SingleMessage(em_id, net);
    sm.start("Hello man!");

    while(true) {}
    return 0;
}

void sigint_handler(int signum) {
    delete logger_ptr;

    char buf[BUF_SIZE];
    size_t offset = 0;

    offset += Logger::push_to_buffer_string(buf + offset, "[DUMMY][SIGINT]");
    offset += Logger::push_to_buffer_time(buf + offset, CLOCK_PROCESS_CPUTIME_ID);
    offset += Logger::push_to_buffer_string(buf + offset, "[");
    offset += Logger::push_to_buffer_int(buf + offset, getpid());
    offset += Logger::push_to_buffer_string(buf + offset, "]");
    offset += Logger::push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);

    _exit(signum);
}
