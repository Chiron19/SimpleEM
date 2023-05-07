#pragma once

#include <string>

#include "network.hpp"

class AlgorithmBase {

protected:

    int em_id;
    Network& net;

public:

    AlgorithmBase(int em_id, Network& net): em_id(em_id), net(net) {}

    virtual void start(std::string message) = 0;

};
