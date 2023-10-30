#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "utils.hpp"

const std::string CONFIG_PATH("./configs/config7.txt"); ///< Path to configuration file
const int STEPS = 10000; ///< Number of times emulator awakens some process

/** @brief Class parsing and saving the configuration from a file
 */
class ConfigParser {

public:

    /** \brief Reads config from the config file ( \p config_path )
     *
     * Config layout is as follows:
     * - First line consists of three words split by whitespace, tun device name,
     *   tun interface address, and tun interface address mask, to be used in
     *   setting up the tun interface.
     * - Second line consists of one number - procs - num of procs to simulate
     * - Next procs lines consists of a string and int each, 
     *   i-th line means the address and port on which i-th proc is listening
     * - Next procs lines consists of procs numbers each, i-th number in
     *   j-th line means the latency from i-th to j-th proc in miliseconds
     *   (i-th number in i-th column should be 0)
     * - Next procs lines consist of at least two words each, i-th line 
     *   starts with the path to i-th program, then the name of the i-th program
     *   and then whitespace-split args to the i-th program.
     * 
     */
    ConfigParser(const std::string& config_path);

    std::string tun_dev_name; ///< Device name for new TUN interface
    std::string tun_addr; ///< Address of the TUN interface
    std::string tun_mask; ///< Mask of the TUN interface subnetwork
    int procs; ///< Number of processes to be emulated by the emulator
    std::vector<std::vector<int>> latency; ///< Matrix of pairwise latencies
    std::vector<std::pair<std::string, int>> addresses; ///< Addresses (in number/dot format) and ports of processes
    std::vector<std::string> program_paths; ///< Paths to programs to be run on every process
    std::vector<std::string> program_names; ///< Names of programs to be run on every process
    std::vector<std::vector<std::string>> program_args; ///< Arguments to be passed to every process

};


ConfigParser::ConfigParser(const std::string& config_path) {
    std::ifstream config(config_path);
    std::istringstream args_stream;
    std::string address, program_path, args_line, arg;
    int port, lat;

    config >> tun_dev_name >> tun_addr >> tun_mask;
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
        }
    }

    std::getline(config, args_line); // Read emptyline 

    for (int i = 0; i < procs*2; ++i) {
        std::getline(config, args_line);
        args_stream = std::istringstream(args_line);
        program_args.push_back(std::vector<std::string>());

        args_stream >> program_path;
        program_paths.push_back(program_path);
        // program_names.push_back(program_name);

        while (args_stream >> arg) {
            // args_stream >> arg;
            // std::cout << arg << std::endl;
            program_args.back().push_back(arg);
        }

        // std::cout << args_stream.str() << std::endl;
        // std::cout << "Num of args: " << program_args[i].size() << std::endl;
        // std::cout << "[config-parser.hpp]" << std::endl;
    }

}
