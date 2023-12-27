#pragma once

#include <unistd.h>

#include "network-helper.hpp"
#include "algorithms/algorithm-base.hpp"


class LoopNetwork: public AlgorithmBase {

public: 

    using AlgorithmBase::AlgorithmBase;

    /** \brief Every node passes the message to the next node after 
     *         receiving it. In total \p loops are done. 0 starts the loop.
     */
    void start(const std::string& message, int loops) {
        
        for (int i = 0; i < loops; ++i) {
            if (em_id == 0 && i == 0) {
                net.send_udp(1, message);
                continue;
            }

            message_t mes = force_receive();
            std::cout << getpid() << " received from " << mes.first << std::endl;
            net.send_udp((em_id + 1) % net.procs, mes.second);
            std::cout << getpid() << " sent to " << (em_id + 1) % net.procs << std::endl;

        }

        if (em_id == 0) {
            message_t mes = force_receive();
            std::cout << getpid() << " received from " << mes.first << std::endl;
        }

    }

};