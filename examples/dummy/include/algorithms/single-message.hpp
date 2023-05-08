#pragma once

#include <string>
#include <assert.h>

#include "network.hpp"
#include "algorithms/algorithm_base.hpp"

class SingleMessage: public AlgorithmBase {

public:

    using AlgorithmBase::AlgorithmBase;

    /** \brief 0 -> 0 , rest nothing
     *  
     */
    void start(const std::string& message) override {

        log_event_proc_cpu_time("Starting");
        if (em_id == 0)
            net.send(1, message);
        if (em_id == 1) {
            while(true) {
                // Just send acknowledgement
                message_t mes = net.receive();
                if (mes.first >= 0) {
                    log_event_proc_cpu_time("Got from %d message: %s", mes.first, mes.second.c_str());
                    break;  
                }
                struct timespec req = (struct timespec){0, 10000};
                nanosleep(&req, nullptr);
            }
        }

        log_event_proc_cpu_time("Broadcast finished");
    }

};


