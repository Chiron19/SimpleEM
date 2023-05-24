#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <string>

#include "config-parser.hpp"

#include "utils.hpp"
#include "logger.hpp"

#include "network/network.hpp"
#include "emulator.hpp"

Logger* logger_ptr = nullptr;
Emulator* em_ptr = nullptr;

void sigint_handler(int signum);

int main() {
	logger_ptr = new Logger("logging_tinyem.txt");

	ConfigParser cp(CONFIG_PATH);
	Network network(cp);
	em_ptr = new Emulator(network, cp);
	
	real_sleep(10 * MILLISECOND); /* Give some time */

	signal(SIGINT, sigint_handler);
	em_ptr->start_emulation(STEPS);

	real_sleep(10 * MILLISECOND); /* Give some time */

	em_ptr->kill_emulation();
	delete logger_ptr;

	return 0;
}


void sigint_handler(int signum) {
	if (em_ptr)
		em_ptr->kill_emulation();
	delete logger_ptr;

	char buf[BUF_SIZE];
    size_t offset = 0;

    offset += Logger::push_to_buffer_string(buf + offset, "[TINYEM][SIGINT]");
    offset += Logger::push_to_buffer_time(buf + offset, CLOCK_MONOTONIC);
	offset += Logger::push_to_buffer_string(buf + offset, "[");
    offset += Logger::push_to_buffer_int(buf + offset, getpid());
    offset += Logger::push_to_buffer_string(buf + offset, "]");
    offset += Logger::push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);

	_exit(signum);
}
