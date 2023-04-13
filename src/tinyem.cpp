#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <string>

#include "config.hpp"

#include "utils.hpp"
#include "proc_control/context.hpp"
#include "proc_control/proc_init.hpp"
#include "network/tun_init.hpp"

#include "network/buffers.hpp"

#define PATH_MAX 128

FILE* logging_fptr;
int children_pids[EXTRA_PROCESSES]; // For signal to kill children

void sigint_handler(int signum);

int main() {
	signal(SIGINT, sigint_handler);
	EMProc emprocs[EXTRA_PROCESSES];

	logging_fptr = open_logging((char*)"tinyem", false);
	log_event("Process %d started, spawning %d children", getpid(), EXTRA_PROCESSES);

	fork_stop_run(EXTRA_PROCESSES, children_pids, PROGRAM_PATH, PROGRAM_ARGV);
	for (em_id_t i = 0; i < EXTRA_PROCESSES; ++i) {
		emprocs[i].em_id = i;
		emprocs[i].pid = children_pids[i];
	}

	int fd = create_tun(TUN_DEV_NAME, TUN_ADDR, TUN_MASK);

	real_sleep(10 * MILLISECOND); /* Give some time */


	/* Main loop */
	while (true) {

		struct timespec ts = (struct timespec){0, 10 * MILLISECOND};

		for (int i = 0; i < EXTRA_PROCESSES; ++i) {

			awake(emprocs[i], ts, fd);

			/* Now move from to_send buffers to apropriate to_receive buffers,
			   with appropriate buf changes  main NETWORKING stuff */

		}


	}

	return 0;
}


void sigint_handler(int signum) {
	kill_all(EXTRA_PROCESSES, children_pids);
	fclose(logging_fptr);

	char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf, "[TINYEM][SIGINT]");
    offset += push_to_buffer_time(buf + offset, CLOCK_REALTIME);
    offset += push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);
}
