#pragma once

#include <string>

#include "dummy.hpp"

class BestEffortBroadcast {

    NetworkInterface& net;

public:
    BestEffortBroadcast(NetworkInterface& net): net(net) {}

    /** \brief Perform Beb algorithm, broadcasting message once if \p em_id is 0
     */
    void start(int em_id, std::string message) {
        if (em_id == 0) {
            // for (const )
        }

        while(true) {
            message = net.receive();
            if (!message.empty()) {
                log_event_proc_cpu_time("Received message: %s", message.c_str());
            }
        }
    }

};