#pragma once

#include <string>
#include <sstream>

#include "network-helper.hpp"

class AlgorithmBase {

protected:

    int em_id;
    NetworkHelper& net;

public:

    AlgorithmBase(int em_id, NetworkHelper& net): em_id(em_id), net(net) {
        std::cout << "[algorithm-base] init " << net.procs << ' ' << net.em_id << std::endl;
    }

    void broadcast(int em_id, const std::string& message);

    message_t force_receive();

protected:

    std::string get_mes_type(const std::string& message) const;
    std::string get_mes_value(const std::string& message) const;

};

void AlgorithmBase::broadcast(int em_id, const std::string& message) {
    for (int other_id = 0; other_id < net.procs; ++other_id) {
        // net.send(other_id, message);
        if (em_id == other_id) continue;
        if (net.send_tcp(other_id, message) < 0) {
            // 
            std::cout << "[algorithm-base] " << em_id << "->" << other_id << " FAIL" << std::endl;
        }
        else 
        std::cout << "[algorithm-base] " << em_id << "->" << other_id << " (" << message << ")" << std::endl;
    }
}

message_t AlgorithmBase::force_receive() {
    while(true) {
        // message_t mes = net.receive();
        std::cout << "[algorithm-base] recv ing " << std::endl;
        std::cout << "ERROR OCCURS" << net.procs << ' ' << net.em_id << std::endl;
        message_t mes = net.receive_tcp();
        std::cout << "[algorithm-base] recv mes: " << mes.first << " " << mes.second << std::endl;
        
        if (mes.first >= 0) {
            std::cout << "[algorithm-base] recv from " << mes.first << " : " << mes.second << std::endl;
            return mes;
        }
        else {
            std::cout << "[algorithm-base] recv fail " << std::endl;
        }
    }
}

std::string AlgorithmBase::get_mes_type(const std::string& message) const {
    std::istringstream mes_stream(message);
    std::string mes_type, mes_value;
    mes_stream >> mes_type >> mes_value;
    return mes_type;
}

std::string AlgorithmBase::get_mes_value(const std::string& message) const {
    std::istringstream mes_stream(message);
    std::string mes_type, mes_value;
    mes_stream >> mes_type >> mes_value;
    return mes_value;
}