#ifndef SIMPLEEM_EMULATOR
#define SIMPLEEM_EMULATOR

#include <vector>

#include "utils.hpp"

#include "proc_control/proc_init.hpp"
#include "proc_control/emproc.hpp"


class Emulator {

    int procs;
    int ni_fd;
    std::vector<EMProc> emprocs;

public:

    Emulator(int procs, int ni_fd): procs(procs), ni_fd(ni_fd) {}

    void build_children();

    void start_emulation();

    void kill_emulation();

};


void Emulator::build_children() {
    int children_pids[procs];
    fork_stop_run(procs, children_pids, PROGRAM_PATH, PROGRAM_ARGV); 

    for (em_id_t em_id = 0; em_id < procs; ++em_id) {
        emprocs.push_back(EMProc(em_id, children_pids[em_id]));
    }
}

void Emulator::start_emulation() {
    
	while (true) {

		struct timespec ts = (struct timespec){0, 10 * MILLISECOND};

        for (em_id_t em_id = 0; em_id < procs; ++em_id) {
            emprocs[em_id].awake(ts, ni_fd);

            /* Now move from to_send buffers to apropriate to_receive buffers,
			   with appropriate buf changes  main NETWORKING stuff */
        }
	}
}

void Emulator::kill_emulation() {
    for (auto& emproc: emprocs) {
        kill(emproc.pid, SIGCONT);
        kill(emproc.pid, SIGINT);
	}
}

#endif