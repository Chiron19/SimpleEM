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
    void start(std::string message) override {

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

// class Broadcast {

//     int em_id;
//     Network& net;

// public:

//     Broadcast(int em_id, Network& net): em_id(em_id), net(net) {}

//     /** \brief Only em_id=0 broadcasts
//      *
//      */
//     void start(std::string message) {

//         log_event_proc_cpu_time("Broadcast started");
//         if (em_id == 0) {
//             for (int other_id = 1; other_id < net.addresses.size(); ++other_id) {
//                 net.send(other_id, message);
//             }
//             int acknowledged = 0;

//             while(true) {
//                 // Just send acknowledgement
//                 message_t mes = net.receive();
//                 if (mes.first >= 0 && mes.second == "OK") {
//                     acknowledged++;
//                     log_event_proc_cpu_time("Received acknowledgment from %d", mes.first);
//                 }
//                 if (acknowledged == net.procs - 1)
//                     break;
//             }
//         }
//         else {
//             while(true) {
//                 // Just send acknowledgement
//                 message_t mes = net.receive();
//                 if (mes.first >= 0) {
//                     net.send(0, "OK");
//                     log_event_proc_cpu_time("Acknowledged message from %d", mes.first);
//                     break;  
//                 }
//             }
//         }
//         log_event_proc_cpu_time("Broadcast finished");
//     }

// };


// class PingPong {

//     int em_id;
//     Network& net;

// public:

//     PingPong(int em_id, Network& net): em_id(em_id), net(net) {}

//     /** \brief Just forward message to the next proc
//      */
//     void start(std::string message, int echos) {
//         if (em_id == 0) {
//             net.send(1, message);
//         }

//         for (int i = 0; i < echos; ++i) {
//             while(true) {
//                 message_t mes = net.receive();
//                 if (mes.first >= 0) {
//                     net.send((em_id + 1) % net.procs, mes.second);
//                     break;
//                 }
//             }
//         }

//         log_event_proc_cpu_time("Program finished");
//     }

// };


// class SingleMessage {

//     int em_id;
//     Network& net;

// public:

//     SingleMessage(int em_id, Network& net): em_id(em_id), net(net) {}

//     /** \brief send message to next
//      *
//      */
//     void start(std::string message) {

//         log_event_proc_cpu_time("Starting");
        
//         net.send((em_id + 1) % net.procs, message);
//         while(true) {
//             // Just send acknowledgement
//             message_t mes = net.receive();
//             if (mes.first >= 0) {
//                 log_event_proc_cpu_time("Got from %d message: %s", mes.first, mes.second.c_str());
//                 break;  
//             }
//         }

//         log_event_proc_cpu_time("Broadcast finished");
//     }

// };
