#pragma once

#include <sys/wait.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>

#include "utils.hpp"
#include "logger.hpp"
#include "config-parser.hpp"
#include "network/network.hpp"
#include "proc_control/emproc.hpp"
#include "proc_control/proc_frame.hpp"

/** @brief Class encapsulating the main logic of the emulator.
 * 
 * Class responsible for overall control of the emulation. It creates
 * specified number of new processes and runs the emulated program on them.
 * It controls the state of those processes (whether they are stopped or
 * running), and decides which one should be awaken (scheduled) next and
 * for how lond. At last, it is responsible for the cleanup after the
 * emulation is finished.
 */
class Emulator {

    int procs; ///< Number of processes being emulated.
    std::vector<EMProc> emprocs; ///< States of each process
    Network& network; ///< Specifies network on which the emulation is being run

public:

    /** @brief Creates an emulation and spawns specified number of new processes.
     * 
     * Creates the emulation by forking the process @ref procs times. 
     * Every newly created process immediately stops itself by the 
     * `raise(SIGSTOP)` call. Objects corresponding to
     * newly created processes are stored in the @ref emprocs vector.
     * 
     * @param network The network on which the emulator runs
     * @param cp Configuration of the emulation
     */
    Emulator(Network& network, const ConfigParser& cp);

    /** @brief Starts the emulation by iteratively scheduling processes.
     * 
     * Performs @p steps times the loop of choosing the next process to 
     * schedule, calculating for what time it should run, awakening it
     * for that time, and later sorting packets sent by it to appropriate  
     * queues.
     * 
     * @param steps Number of awakenings to perform
     */
    void start_emulation(int steps);

    /** @brief Kills all spawned processes 
     * 
     * Kills all spawned processes, effectively ending the emulation.
     */
    void kill_emulation();

private:

    /** @brief Forks the process and initializes child processes
     * 
     * Creates @ref procs new processes that execute
     * the @ref child_init function. Saves process ids of newly
     * created processes in the @p pids array.
     * 
     * @param pids Array in which to store process ids of new processes
     * @param cp Configuration to be used for new processes initialization
     */
    void fork_stop_run(int* pids, const ConfigParser& cp);

    /** @brief Extracts the name of the program from its path
     * 
     * Extracts the name of the program from its path. If the path
     * contains '/' character, the name is everything after the last
     * occurence of that character. Otherwise, the name is the entire
     * path.
     * 
     * @param filePath Path to the program
     * @return Name of the program
     */
    const char* extractProgramName(const char* filePath);

    /** @brief Executes the program specified by @p program_path
     * 
     * Executes the program specified by @p program_path with arguments
     * specified by @p program_args. The program is executed using the
     * `system()` call.
     * 
     * @param program_path Path to the program to be executed
     * @param program_args Arguments to be passed to the program
     */
    void executeProgram(const std::string& program_path, const std::vector<std::string>& program_args);

    /** @brief Initialize the child process.
     * 
     * Start by doing the `raise(SIGSTOP)` call. After the
     * process is awaken (using the `SIGCONT` signal), it
     * executes the program specified by @p program_path.
     * 
     * @param program_path Path to the program to be executed on this process
     * @param program_name Name of the program to be executed on this process
     * @param program_args Arguments to be passes to the program
     */
    void child_init( 
        const std::string& program_path,
        // const std::string& program_name, 
        const std::vector<std::string>& program_args);

    /** @brief Chooses next process to be scheduled for awakening
     * 
     * Loops through all te processes and choses the one which was executed
     * for the least amount of time.
     * 
     * @return Id of the process to be scheduled next
     */
    em_id_t choose_next_proc() const;

    /** @brief Returns the longest time @p em_id can run 
     *         without violating correctness
     *
     * Every process has its virtual clock saved, returns 
     * minimum (over all processes p != @p em_id ) of 
     * \code{}
     * p->virtual_clock - em_id->virtual_clock + network.get_latency(p, em_id)
     * \endcode
     * @param em_id Process which will be run
     * @return The maximum possible time to run
     */
    struct timespec get_time_interval(em_id_t em_id) const;

    /** @brief Moves packets sent by @p em_id to appropriate in queues of 
     *         receiving processes.
     * 
     * For every packet sent by the @p em_id, this function moves it to 
     * a queue of packets awaiting to be received by appropriate other process.
     * The function increases the timestamp of the packet by the pairwise
     * latency between those two processes - so that the packet will be 
     * received at proper time.
     * 
     * @param em_id Id of the process whose out packets need to be moved
     */
    void schedule_sent_packets(em_id_t em_id);

};

Emulator::Emulator(Network& network, const ConfigParser& cp): network(network) {
    procs = network.get_procs();   

    int children_pids[procs];
    fork_stop_run(children_pids, cp); 

    for (em_id_t em_id = 0; em_id < procs; ++em_id) {
        emprocs.push_back(EMProc(em_id, children_pids[em_id]));
    }

    logger_ptr->log_event("Emulator created with %d processes", procs);                  
}

void Emulator::start_emulation(int steps) {
    em_id_t em_id;
    struct timespec ts;
    
    for (int loop = 0; loop < steps; ++loop) {
        em_id = choose_next_proc();
        ts = get_time_interval(em_id);
        emprocs[em_id].awake(ts, network);
        // printf("[emulator.hpp]em_id : %d\n", em_id);
        schedule_sent_packets(em_id);
	}

}

void Emulator::kill_emulation() {
    int wstatus;

    Logger::print_string_safe("KILLING EMULATION\n");
    for (auto& emproc: emprocs) {
        kill(emproc.pid, SIGCONT);
        kill(emproc.pid, SIGINT);
        // kill(emproc.pid, SIGKILL);
        printf("KILLING %d\n", emproc.pid);
        if (waitpid(emproc.pid, &wstatus, 0) == -1) {
            Logger::print_string_safe("WAITPID FAILED!\n");
        }
        // kill(emproc.pid, SIGTERM);
	}
	Logger::print_string_safe("EMULATION KILLED\n");
}

void Emulator::fork_stop_run(int* pids, const ConfigParser& cp) {
    int status;

    for (int i = 0; i < procs; ++i) {
		status = fork();

		if (status == -1) {
            Logger::print_string_safe("[ERROR] Forking error!");
			exit(1);
		}
		else if (status == 0) { /* This is a child process */
			child_init(
                cp.program_paths[i],
                // cp.program_names[i],
                cp.program_args[i]);
		}
		else { /* This is a parent process, new_id is set to child's pid */
			pids[i] = status;
		}
	}
}

const char* Emulator::extractProgramName(const char* filePath) {
    const char* fileName = std::strrchr(filePath, '/');

    // If '/' is found, move one character ahead to get the file name
    if (fileName != nullptr) {
        return fileName + 1;
    }

    // If '/' is not found, the entire path is the file name
    return filePath;
}

void Emulator::executeProgram(const std::string& program_path, const std::vector<std::string>& program_args) {
    std::ostringstream cmd;
    cmd << program_path;

    for (const auto& arg : program_args) {
        cmd << ' ' << arg;
    }

    int result = system(cmd.str().c_str());
    if (result == 0) {
        // The command executed successfully
    } else {
        // The command failed
    }
}

void Emulator::child_init(const std::string& program_path, const std::vector<std::string>& program_args) {
    int status;
    // Dynamically allocate memory for program_argv
    char** program_argv = new char*[program_args.size() + 2];

    program_argv[0] = const_cast<char*>(program_path.c_str());
    // Logger::print_string_safe(program_path + " ");
    for (size_t i = 0; i < program_args.size(); ++i) {
        // Convert each string to a non-const char*
        program_argv[i + 1] = const_cast<char*>(program_args[i].c_str());
        // Logger::print_string_safe(program_args[i] + " ");
    }
    program_argv[program_args.size() + 1] = nullptr;
    // Logger::print_string_safe("\n");

    std::ifstream file(program_path);
    if (program_path == "java") {
        try
        {
            executeProgram(program_path, program_args);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
        delete[] program_argv; // Free the allocated memory after execve 
        return;
    }

    if (!file.good()) {
        char buf[] = "[WARNING] File does not exist: ";
        strcat(buf, program_path.c_str());
        strcat(buf, "\n");
        Logger::print_string_safe(buf);
    }

    if ((status = raise(SIGSTOP)) != 0) {
        Logger::print_string_safe("[ERROR] raise(SIGSTOP) failed!\n");
        perror("");
        delete[] program_argv; // Free the allocated memory before returning
        return;
    }

    if ((status = execve(program_path.c_str(), program_argv, nullptr)) == -1) {
        Logger::print_string_safe("[ERROR] execve() failed!\n");
        perror("");
        delete[] program_argv; // Free the allocated memory before returning
        return;
    }

    delete[] program_argv; // Free the allocated memory after execve 
}

em_id_t Emulator::choose_next_proc() const {
    em_id_t earliest_emproc = 0;
    for (em_id_t em_id = 1; em_id < procs; ++em_id) {
        if (emprocs[em_id].virtual_clock < emprocs[earliest_emproc].virtual_clock)
            earliest_emproc = em_id;
    }
    return earliest_emproc;
}

struct timespec Emulator::get_time_interval(em_id_t em_id) const {
    struct timespec result_ts = 2 * network.get_max_latency();

    for (em_id_t other_proc = 0; other_proc < procs; ++other_proc) {
        if (other_proc == em_id)
            continue;
        result_ts = std::min(result_ts, emprocs[other_proc].virtual_clock - 
            emprocs[em_id].virtual_clock + network.get_latency(other_proc, em_id));
    }
    return result_ts;
}

void Emulator::schedule_sent_packets(em_id_t em_id) {
    while(!emprocs[em_id].out_packets.empty()) {
        Packet packet = emprocs[em_id].out_packets.top();

        // printf("[emulator.hpp] Out_packets: ------- em_id: %d\n", em_id);

        emprocs[em_id].out_packets.pop();

        if (packet.get_version() != 4)
            continue; // FIXME temporary fix of random packets

        em_id_t dest_em_id = network.get_em_id(packet.get_dest_addr());
        packet.increase_ts(network.get_latency(em_id, dest_em_id));

        // packet.set_dest_addr(network.get_inter_addr());
        // packet.set_source_addr(network.get_addr(em_id));

        /* Modified from UDP to TCP */
        // printf("network inter addr: %s\n", network.get_inter_addr().c_str());
        // printf("network addr 0: %s\n", network.get_addr(0).c_str());
        // printf("network addr 1: %s\n", network.get_addr(1).c_str());
        // printf("network addr 2: %s\n", network.get_addr(2).c_str());
        
        // std::cout << "packet size  :" << packet.get_size() << std::endl;
        // std::cout << "packet buffer:" << packet.get_buffer() << std::endl;
        // std::cout << "Emulator here" << std::endl;
        packet.set_dest_addr_tcp(network.get_inter_addr());
        packet.set_source_addr_tcp(network.get_addr(em_id));

        packet.set_dest_addr_tcp(network.get_inter_addr());
        packet.set_source_addr_tcp(network.get_addr(em_id));

    // printf("[emulator.hpp] packet MODIFIED: %s (%d) -> %s (%d)\n", packet.get_source_addr().c_str(), em_id, packet.get_dest_addr().c_str(), dest_em_id); 
    // dump(packet.get_buffer(), packet.get_size());

        logger_ptr->print_string_safe(packet.get_buffer());

        emprocs[dest_em_id].in_packets.push(packet);
    }
}
