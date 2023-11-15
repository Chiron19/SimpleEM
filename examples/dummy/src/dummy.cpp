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
#include <iostream>
#include <fstream>
#include <string>

#include "network-helper.hpp"
#include "tcp-peer.hpp"
#include "test.hpp"
#include "test2.hpp"
#include "algorithms/loop-network.hpp"
#include "algorithms/single-message.hpp"
#include "algorithms/byzantine-reliable-broadcast.hpp"

#include "logger.hpp"
#include "utils.hpp"

void signal_handler(int signum);

Logger* logger_ptr = nullptr;
int em_id;

bool fileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}

int main(int argc, char* argv[]) {
    // arguements: em_id configPath
    em_id = std::stoi(std::string(argv[1]));
    logger_ptr = new Logger("logging_dummy_" + std::to_string(em_id) + ".txt");
    signal(SIGINT, signal_handler);
    
    std::string configPath = std::string(argv[2]);
    if (!fileExists(configPath)) {
        std::cout << "File does not exist at path: " << configPath << std::endl;
        return 0;
    }

    // Create 2 instances for send/recv
    NetworkHelper net_send = NetworkHelper(em_id, std::string(argv[2]));
    NetworkHelper net_recv = NetworkHelper(em_id, std::string(argv[2]));
    logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, "Start of %d", em_id);

    std::cout << "[dummy] em_id: " << em_id << std::endl;
    // for (int i = 0; i < net.addresses.size(); i++) {
    //     std::cout << net.addresses[i].first << " " << net.addresses[i].second << std::endl;
    // }
    // printf("[dummy] Ready\n");

    // ByzantineReliableBroadcast brb = ByzantineReliableBroadcast(em_id, net);
    // brb.start("This is a broadcast!");
    // LoopNetwork ln = LoopNetwork(em_id, net);
    // ln.start("Hello!", 2);
    // SingleMessage sm = SingleMessage(em_id, net);
    // sm.start("Hello man!");
    // std::cout << "[dummy] " << net_send.procs << ' ' << net_send.em_id << std::endl;
    // TCPpeer tcp_peer = TCPpeer(em_id, net_send, net_recv);

    // std::cout << "[dummy] tcp_peer created" << std::endl;
    // tcp_peer.tcp_thread();

    std::cout << "[dummy] " << net_send.procs << ' ' << net_send.em_id << std::endl;
    // Test test_instance = Test(em_id, net_send, net_recv); // Test.hpp
    // test_init(em_id, net_send, net_recv); // Test2.hpp

    std::cout << "[dummy] tcp_peer created" << std::endl;
    // test_instance.tcp_thread(&test_instance); // Test.hpp
    // tcp_thread(net_send, net_recv); // Test2.hpp
    
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
