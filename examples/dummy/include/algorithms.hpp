#pragma once

#include <string>
#include <assert.h>

#include "dummy.hpp"

class Broadcast {

    int em_id;
    NetworkInterface& net;

public:

    Broadcast(int em_id, NetworkInterface& net): em_id(em_id), net(net) {}

    /** \brief Only em_id=0 broadcasts
     *
     */
    void start(std::string message) {

        log_event_proc_cpu_time("Broadcast started");
        if (em_id == 0) {
            for (int other_id = 0; other_id < net.addresses.size(); ++other_id) {
                net.send(other_id, message);
            }
            int acknowledged = 0;

            while(true) {
                // Just send acknowledgement
                message_t mes = net.receive();
                if (mes.first >= 0) {
                    if (mes.second == "OK") {
                        acknowledged++;
                        log_event_proc_cpu_time("Received acknowledgment from %d", mes.first);
                    }
                    else {
                        net.send(mes.first, "OK");
                        log_event_proc_cpu_time("Acknowledged message from %d", mes.first);
                    }
                }
                if (acknowledged == net.procs)
                    break;
                
            }
        }
        else {
            while(true) {
                // Just send acknowledgement
                message_t mes = net.receive();
                if (mes.first >= 0) {
                    net.send(mes.first, "OK");
                    log_event_proc_cpu_time("Acknowledged message from %d", mes.first);
                    break;  
                }
            }
        }
        log_event_proc_cpu_time("Broadcast finished");
    }

};


class SingleMessage {

    int em_id;
    NetworkInterface& net;

public:

    SingleMessage(int em_id, NetworkInterface& net): em_id(em_id), net(net) {}

    /** \brief em_id=0 sends one message to em_id=1
     *
     */
    void start(std::string message) {

        log_event_proc_cpu_time("Starting");
        if (em_id == 0) {
            net.send(1, message);
        }
        else {
            while(true) {
                // Just send acknowledgement
                message_t mes = net.receive();
                if (mes.first >= 0) {
                    log_event_proc_cpu_time("Got from %d message: %s", mes.first, mes.second.c_str());
                    break;  
                }
            }
        }
        log_event_proc_cpu_time("Broadcast finished");
    }

};
