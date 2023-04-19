#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <string>

#include "config.hpp"

#include "utils.hpp"

#include "emulator.hpp"

#include "network/tun_init.hpp"

#define PATH_MAX 128

FILE* logging_fptr;
Emulator* em_ptr;

void sigint_handler(int signum);

int main() {
	signal(SIGINT, sigint_handler);

	logging_fptr = open_logging((char*)"tinyem", false);
	log_event("Process %d started, spawning %d children", getpid(), EXTRA_PROCESSES);

	int fd = create_tun(TUN_DEV_NAME, TUN_ADDR, TUN_MASK);

	em_ptr = new Emulator(EXTRA_PROCESSES, fd);
	em_ptr->build_children();

	real_sleep(10 * MILLISECOND); /* Give some time */

	em_ptr->start_emulation();

	return 0;
}


void sigint_handler(int signum) {
	em_ptr->kill_emulation();
	fclose(logging_fptr);

	char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf, "[TINYEM][SIGINT]");
    offset += push_to_buffer_time(buf + offset, CLOCK_MONOTONIC);
    offset += push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);
}
