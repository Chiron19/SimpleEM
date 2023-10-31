#pragma once

#include <unistd.h>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <fstream>

#include "network-helper.hpp"
#include "algorithms/algorithm-base.hpp"

class SingleMessage: public AlgorithmBase {
protected:

    int inited;

public: 

    using AlgorithmBase::AlgorithmBase;

    std::string getLocalTime() {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H-%M-%S");

        return oss.str();
    }

    /** \brief 0->1
     */
    void start(const int sender_or_receiver, const std::string& message) {

        if (sender_or_receiver) { // Sender
            // std::cout << "[single-message] sender" << std::endl;
            if (em_id == 0 && !inited) {
                while (net.send_tcp(1, message) < 0) {
                    // std::cout << "[single-message] init fail!" << std::endl;
                    std::cout << "[single-message] procs:" << net.procs << std::endl;
                    if (inited) return;
                    // sleep(1);
                }
                std::cout << "[single-message] message sent!\n" << std::endl;
                inited = 1;
                std::cout << "[single-message] inited:" << inited << std::endl;
            }

            if (net.inbox.size() > em_id && !net.inbox[em_id].empty()) {
                message_t mes = net.inbox[em_id].top();
                std::cout << "[single-message] replying" << mes.first << ":" << mes.second << std::endl;
                net.inbox[em_id].pop();
                std::string message_reply = getLocalTime();
                while (net.send_tcp(mes.first, message_reply) < 0);
                std::cout << "[single-message] message sent!\n" << std::endl;
            }
        }
        else { // Receiver
            
            message_t mes = force_receive();
            net.inbox[em_id].push(mes);
            std::cout << "[single-message] message: " << mes.second << std::endl;
            Logger::print_string_safe(
                std::to_string(em_id) + " GOT FROM " + std::to_string(mes.first) + " MESSAGE: " + mes.second + "\n\n");
            
        }

    }

};

