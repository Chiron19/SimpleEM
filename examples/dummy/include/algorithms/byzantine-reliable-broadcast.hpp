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
        {
            broadcast(em_id, type_send + " " + message);
        }   

        // Logger::print_string_safe("[byzantine] It is here.\n");

        while(true) {
            if (em_id == 0 && !sentecho) {
                sentecho = true;
                echos[em_id] = true;
                std::cout << "[byzantine] " << ": 000" << std::endl;
                broadcast(em_id, type_echo + " " + message);
                std::cout << "[byzantine] " << em_id << ": echo\n" << std::endl;
            }
            std::cout << "[byzantine] " << em_id << " recv status:";
            if (sentecho)  std::cout << " " << "sentecho"; 
            if (sentready) std::cout << " " << "sentready"; 
            std::cout << std::endl;
            // std::cout << "[byzantine] " << "echo :" ;
            // for (int other_id = 0; other_id < net.procs; ++other_id) 
            //     printf(echos[other_id] ? "#" : "-");
            // printf("\n");
            // std::cout << "[byzantine] " << "ready:" ;
            // for (int other_id = 0; other_id < net.procs; ++other_id) 
            //     printf(readys[other_id] ? "#" : "-");
            // printf("\n");

            printf("------%d receive begin\n", em_id);
            // mes_received = net.receive_tcp();
            mes_received = force_receive();
            mes_type = get_mes_type(mes_received.second);
            mes_value = get_mes_value(mes_received.second);
            printf("------%d receive finish\n", em_id);

            if (mes_type == type_send && !sentecho && mes_received.first == 0) {
                // should we force mes_received.first==0?
                // fix the initial sender as em_id == 0
                sentecho = true;
                echos[em_id] = true;
                std::cout << "[byzantine] " << ": 000" << std::endl;
                broadcast(em_id, type_echo + " " + mes_value);
            }    
            if (mes_type == type_echo) {
                if (!echos[mes_received.first]) 
                    echoed++;
                echos[mes_received.first] = true;
                std::cout << "[byzantine] " << em_id << ": has echoed " << echoed << std::endl;
            }
            if (mes_type == type_send) {
                if (!readys[mes_received.first])
                    readied++;
                readys[mes_received.first] = true;
                std::cout << "[byzantine] " << em_id << ": has readied " << readied << std::endl;
            }
            if (mes_type != type_send && mes_type != type_echo && mes_type != type_ready) {
                std::cout << "UNIDENTIFIED MESSAGE TYPE: " << mes_type 
                          << " MESSAGE VALUE: " << mes_value
                          << " SENDER ID: " << mes_received.first
                          << std::endl;
                // exit(1);
                raise(SIGSTOP);
            }

            if (echoed >= (net.procs + f) / 2 && !sentecho) {
                sentecho = true;
                echos[em_id] = true;
                std::cout << "[byzantine] " << ": 111" << std::endl;
                broadcast(em_id, type_echo + " " + mes_value);
                continue;
            }
            if (readied > f && !sentecho) {
                sentecho = true;
                echos[em_id] = true;
                std::cout << "[byzantine] " << ": 222" << std::endl;
                broadcast(em_id, type_echo + " " + mes_value);
                continue;
            }
            if (echoed >= (net.procs + f) / 2 && !sentready) {
                sentready = true;
                readys[em_id] = true;
                std::cout << "[byzantine] " << ": 333" << std::endl;
                broadcast(em_id, type_ready + " " + mes_value);
                continue;
            }
            if (readied > f && !sentready) {
                sentready = true;
                readys[em_id] = true;
                std::cout << "[byzantine] " << ": 444" << std::endl;
                broadcast(em_id, type_ready + " " + mes_value);
                continue;
            }
            if (readied > 2 * f && !delivered) {
                delivered = true;
                logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID,
                    "DELIVERING MESSAGE: %s", mes_value.c_str());
                std::cout << "[byzantine] " << em_id << " DELIVERING MESSAGE:" << mes_value << std::endl;
                // raise(SIGSTOP);
            }

        }


    }

};