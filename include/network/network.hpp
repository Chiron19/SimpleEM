#ifndef SIMPLEEM_NETWORK
#define SIMPLEEM_NETWORK

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

class Network {

    int procs;
    std::vector<std::vector<int>> latency;
    std::vector<std::pair<std::string, int>> addresses;
    struct timespec max_latency;

public:

    /** \brief Load network configuration from config file
     * 
     * Load network configuration from file \p config_path
     * The expected file format is as follows:
     * - first line consists of single number 'procs' - number of processes
     * to emulate
     * - next 'procs' lines consist of two entries, i-th line consists of 
     * the address under which the i-th process receives packets (in number 
     * dot form) and the port.
     * - next 'procs' lines consist of 'procs' numbers each. The j-th number
     * in i-th line is the latency, when sending packet from i-th node to j-th
     * node in miliseconds. Values on diagonal of the matrix have 
     * to be equal to 0.
     */
    Network(std::string config_path);

    struct timespec get_latency(int em_id1, int em_id2) const;

    struct timespec get_max_latency() const;

    int get_procs() const;

    int get_em_id(const std::string& address) const;

};

Network::Network(std::string config_path) {
    std::ifstream config(config_path);
    std::string line, address;
    int port, lat;
    max_latency = timespec{0, 0};

    config >> procs;

    for (int i = 0; i < procs; ++i) {
        config >> address >> port;
        addresses.push_back(std::make_pair(address, port));
    }

    for (int i = 0; i < procs; ++i) {
        latency.push_back(std::vector<int>());

        for (int j = 0; j < procs; ++j) {
            config >> lat;
            if (i == j && lat != 0) {
                std::cout << "NETWORK CONFIG NON-ZERO AT DIAGONAL" << std::endl;
                exit(1);
            }
            latency.back().push_back(lat * MILLISECOND); // latency is given in miliseconds
            if (ts_from_nano(latency.back().back()) > max_latency)
                max_latency = ts_from_nano(latency.back().back());
        }
    }
}

struct timespec Network::get_latency(int em_id1, int em_id2) const {
    return ts_from_nano(latency[em_id1][em_id2]);
}

struct timespec Network::get_max_latency() const {
    return max_latency;
}

int Network::get_procs() const {
    return procs;
}

int Network::get_em_id(const std::string& address) const {
    for (int em_id = 0; em_id < procs; ++em_id) {
        if (address == addresses[em_id].first)
            return em_id;
    }
    return -1; // address unavailable
}

#endif // SIMPLEEM_NETWORK