#pragma once

#include <unistd.h>

#include "network-helper.hpp"
#include "algorithms/algorithm-base.hpp"


class SingleMessage: public AlgorithmBase {

public: 

    using AlgorithmBase::AlgorithmBase;

    /** \brief 0->1
     */
    void start(const std::string& message) {
        

        if (em_id == 0)
            net.send(1, message);

        // if (em_id == 1) {
        //     message_t mes = force_receive();
        //     std::cout << "GOT FROM " << mes.first << " MESSAGE: " << mes.second << std::endl;
        // }

    }

};