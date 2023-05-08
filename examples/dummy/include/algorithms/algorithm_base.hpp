#pragma once

#include <string>
#include <sstream>

#include "network.hpp"

class AlgorithmBase {

protected:

    int em_id;
    Network& net;

public:

    AlgorithmBase(int em_id, Network& net): em_id(em_id), net(net) {}

    virtual void start(const std::string& message) = 0;

    void broadcast(const std::string& message);

    message_t force_receive();

protected:

    std::string get_mes_type(const std::string& message) const;
    std::string get_mes_value(const std::string& message) const;

};

void AlgorithmBase::broadcast(const std::string& message) {
    for (int other_id = 0; other_id < net.procs; ++other_id) {
        net.send(other_id, message);
    }
}

message_t AlgorithmBase::force_receive() {
    while(true) {
        message_t mes = net.receive();
        if (mes.first >= 0)
            return mes;
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