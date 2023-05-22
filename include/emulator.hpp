#pragma once

#include <vector>
#include <iostream>
#include <string>

#include "utils.hpp"
#include "config-parser.hpp"

#include "network/network.hpp"

#include "proc_control/emproc.hpp"

#include "proc_frame.hpp"

class Emulator {

    int procs;
    std::vector<EMProc> emprocs;
    Network& network;

public:

    Emulator(Network& network, const ConfigParser& cp);

    void start_emulation(int steps);

    void kill_emulation();

    /*
     * Creates procs_num new processes that want to run
     * the program specified in the 'config.h' file, but
     * just before they do, they raise(SIGSTOP). Their
     * process ids are saved in pids array
     */
    void fork_stop_run(int* pids, const ConfigParser& cp);

    void child_init( 
        const std::string& program_path,
        const std::string& program_name, 
        const std::vector<std::string>& program_args);

private:
    em_id_t choose_next_proc() const;

    /** \brief Returns the longest time \p em_id can run 
     *         without violating correctness
     *
     * Every process has its 'proc_runtime' saved, returns 
     * minimum (over all processes p != \p em_id ) of 
     * p->proc_runtime - em_id ->proc_runtime + network.get_latency(p, em_id)
     * 
     * \param em_id Process which will be run
     * \return The maximum possible time to run
     */
    struct timespec get_time_interval(em_id_t em_id) const;

    void schedule_sent_packets(em_id_t em_id);

};

Emulator::Emulator(Network& network, const ConfigParser& cp): network(network) {
    procs = network.get_procs();   

    int children_pids[procs];
    fork_stop_run(children_pids, cp); 

    for (em_id_t em_id = 0; em_id < procs; ++em_id) {
        emprocs.push_back(EMProc(em_id, children_pids[em_id]));
    }

    log_event("Emulator created with %d processes", procs);                  
}

void Emulator::start_emulation(int steps) {
    em_id_t em_id;
    struct timespec ts;
    
    for (int loop = 0; loop < steps; ++loop) {
        em_id = choose_next_proc();
        ts = get_time_interval(em_id);
        emprocs[em_id].awake(ts, network);
        schedule_sent_packets(em_id);
	}

}

void Emulator::kill_emulation() {
    for (auto& emproc: emprocs) {
        kill(emproc.pid, SIGCONT);
        kill(emproc.pid, SIGINT);
	}
	log_event("Emulation killed.");
}

em_id_t Emulator::choose_next_proc() const {
    em_id_t earliest_emproc = 0;
    for (em_id_t em_id = 1; em_id < procs; ++em_id) {
        if (emprocs[em_id].proc_runtime < emprocs[earliest_emproc].proc_runtime)
            earliest_emproc = em_id;
    }
    return earliest_emproc;
}

struct timespec Emulator::get_time_interval(em_id_t em_id) const {
    struct timespec result_ts = 2 * network.get_max_latency();

    for (em_id_t other_proc = 0; other_proc < procs; ++other_proc) {
        if (other_proc == em_id)
            continue;
        result_ts = std::min(result_ts, emprocs[other_proc].proc_runtime - 
            emprocs[em_id].proc_runtime + network.get_latency(other_proc, em_id));
    }
    return result_ts;
}

void Emulator::schedule_sent_packets(em_id_t em_id) {
    while(!emprocs[em_id].out_packets.empty()) {
        Packet packet = emprocs[em_id].out_packets.top();
        emprocs[em_id].out_packets.pop();

        if (packet.get_version() != 4)
            continue; // FIXME temporary fix of random packets

        em_id_t dest_em_id = network.get_em_id(packet.get_dest_addr());
        packet.increase_ts(network.get_latency(em_id, dest_em_id));

        packet.set_dest_addr(network.get_inter_addr());
        packet.set_source_addr(network.get_addr(em_id));

        emprocs[dest_em_id].in_packets.push(packet);
    }
}

void Emulator::fork_stop_run(int* pids, const ConfigParser& cp) {
    int status;

    for (int i = 0; i < procs; ++i) {
		status = fork();

		if (status == -1) {
			printf("[TINYEM][ERROR] Forking error, exiting\n");
			exit(1);
		}
		else if (status == 0) { /* This is a child process */
			child_init(
                cp.program_paths[i],
                cp.program_names[i],
                cp.program_args[i]);
		}
		else { /* This is a parent process, new_id is set to child's pid */
			pids[i] = status;
		}
	}
}

void Emulator::child_init(
        const std::string& program_path,
        const std::string& program_name, 
        const std::vector<std::string>& program_args) {
	int status;
	const char *program_argv[program_args.size() + 2];
    program_argv[0] = program_name.c_str();
    for (int i = 0; i < program_args.size(); ++i) {
        program_argv[i + 1] = program_args[i].c_str();
    }
    program_argv[program_args.size() + 1] = nullptr;

	if ((status = raise(SIGSTOP)) != 0) {
		printf("[TINYEM] Process %d couldnt SIGSTOP itself\n", getpid());
		return;
	}
	if ((status = execve(program_path.c_str(), (char * const*)program_argv, NULL)) == -1) {
		printf("[TINYEM] Process %d couldnt execute program\n", getpid());
		return;
	}

	printf("[TINYEM] Process %d finished all the work\n", getpid());
	return;
}
