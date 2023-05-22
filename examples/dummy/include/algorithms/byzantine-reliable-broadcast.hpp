#pragma once

#include "network-helper.hpp"
#include "algorithms/algorithm-base.hpp"


class ByzantineReliableBroadcast: public AlgorithmBase {

    const std::string type_send = std::string("SEND");
    const std::string type_echo = std::string("ECHO");
    const std::string type_ready = std::string("READY");
    bool sentecho, sentready, delivered;
    std::vector<bool> echos; // we remember if echoed, and not what echoed
    std::vector<bool> readys; // we remember if echoed, and not what echoed

public: 

    using AlgorithmBase::AlgorithmBase;

    /** \brief 0 broadcasts, but is also part of senders
     */
    void start(const std::string& message) {
        int f = (net.procs - 1) / 3;
        message_t mes_received;
        std::string mes_type, mes_value;
        sentecho = false;
        sentready = false;
        delivered = false;
        echos = std::vector<bool>(net.procs, false);
        readys = std::vector<bool>(net.procs, false);
        int echoed = 0, readied = 0;
        

        if (em_id == 0)
            broadcast(type_send + " " + message);

        while(true) {
            mes_received = force_receive();
            mes_type = get_mes_type(mes_received.second);
            mes_value = get_mes_value(mes_received.second);

            if (mes_type == type_send && !sentecho && mes_received.first == 0) {
                // should we force mes_received.first==0?
                // for now we do not do this.
                sentecho = true;
                broadcast(type_echo + " " + mes_value);
            }
            else if (mes_type == type_echo) {
                if (!echos[mes_received.first])
                    echoed++;
                echos[mes_received.first] = true;
            }
            else if (mes_type == type_ready) {
                if (!readys[mes_received.first])
                    readied++;
                readys[mes_received.first] = true;
            }
            else {
                std::cout << "UNIDENTIFIED MESSAGE TYPE: " << mes_type 
                          << " MESSAGE VALUE: " << mes_value
                          << " SENDER ID: " << mes_received.first
                          << std::endl;
                // exit(1);
                raise(SIGSTOP);
            }

            if (echoed > (net.procs + f) / 2 && !sentready) {
                sentready = true;
                broadcast(type_ready + " " + mes_value);
            }
            if (readied > f && !sentready) {
                sentready = true;
                broadcast(type_ready + " " + mes_value);
            }
            if (readied > 2 * f && !delivered) {
                delivered = true;
                log_event_proc_cpu_time("DELIVERING MESSAGE: %s", mes_value.c_str());
            }

        }


    }

};