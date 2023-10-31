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
#include "algorithms/single-message-duplex.hpp"
#include "algorithms/byzantine-reliable-broadcast.hpp"

#include "tcp_file.hpp"
#include "logger.hpp"
#include "utils.hpp"

void signal_handler(int signum);

Logger* logger_ptr = nullptr;
int em_id, sender_or_receiver;
// Sender: 1, Receiver: 0

int main(int argc, char* argv[]) {
    em_id = std::stoi(std::string(argv[1]));
    sender_or_receiver = (std::stoi(std::string(argv[2]))) ? 1 : 0;

    std::cout << "[dummy] em_id: " << em_id ;
    std::cout << (sender_or_receiver ? ", Sender" : ", Receiver") << std::endl;

    if (sender_or_receiver)
        logger_ptr = new Logger("logging_dummy_" + std::to_string(em_id) + ".txt");
    else 
        logger_ptr = new Logger("logging_dummy_" + std::to_string(em_id) + ".txt");
    
    signal(SIGINT, signal_handler);
    
    NetworkHelper net = NetworkHelper(em_id, std::string(argv[3]));

    logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, "Start of %d, %d", em_id, sender_or_receiver);

    // // ByzantineReliableBroadcast brb = ByzantineReliableBroadcast(em_id, net);
    // // brb.start("This is a broadcast!");
    // // LoopNetwork ln = LoopNetwork(em_id, net);
    // // ln.start("Hello!", 2);
    for(;;) {
        SingleMessage sm = SingleMessage(em_id, net);
        sm.start(sender_or_receiver, "Hello man!");
    }

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
