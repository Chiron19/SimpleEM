#pragma once

#include <unistd.h>

#include "network-helper.hpp"
#include "algorithms/algorithm-base.hpp"


class SingleMessage: public AlgorithmBase {

public: 

    using AlgorithmBase::AlgorithmBase;

    /** \brief 0->1
     */
    void start(const std::string& message) {

        if (em_id == 0) {
            while (net.send_tcp(1, message) < 0);
            // net.send_tcp(1, message);
            std::cout << "[single-message] message sent!" << std::endl;
            // net.send(2, message);
        }

        if (em_id == 1){
            message_t mes = force_receive();
            // message_t mes = net.receive_tcp();
            std::cout << "[single-message] message: " << mes.second << std::endl;
            Logger::print_string_safe(
                std::to_string(em_id) + " GOT FROM " + std::to_string(mes.first) + " MESSAGE: " + mes.second + "\n");
        }

    }

};