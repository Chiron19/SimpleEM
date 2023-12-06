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

void signal_handler(int signum);

int main(int argc, const char** argv) {
	logger_ptr = new Logger("logging_tinyem.txt");

	if (argc == 2) {
		CONFIG_PATH = std::string(argv[1]);
		// std::cout << CONFIG_PATH << std::endl;
	}
	if (argc > 2) {
		Logger::print_string_safe("Usage: ./tinyem (config_file_name_with_extension)\n");
	}
	ConfigParser cp((const std::string) CONFIG_PATH);
	Network network(cp);
	em_ptr = new Emulator(network, cp);
	
	real_sleep(10 * MILLISECOND); /* Give some time */

	signal(SIGINT, signal_handler);
	signal(SIGSEGV, signal_handler);
	em_ptr->start_emulation(STEPS);

	Logger::print_string_safe("EMULATION FINISHED\n");
	em_ptr->kill_emulation();
	delete logger_ptr;

	return 0;
}


void signal_handler(int signum) {

	char buf[BUF_SIZE];
    size_t offset = 0;

    offset += Logger::push_to_buffer_string_safe(buf + offset, "[TINYEM]");

	switch (signum) {
    case SIGINT:
        offset += Logger::push_to_buffer_string_safe(buf + offset, "[SIGINT]");
        break;
    case SIGSEGV:
        offset += Logger::push_to_buffer_string_safe(buf + offset, "[SIGSEGV]");
        break;
    default:
        offset += Logger::push_to_buffer_string_safe(buf + offset, "[UNEXPECTED SIGNAL]");
        break;
    }

    offset += Logger::push_to_buffer_time_safe(buf + offset, CLOCK_MONOTONIC);
	offset += Logger::push_to_buffer_string_safe(buf + offset, "[");
    offset += Logger::push_to_buffer_int_safe(buf + offset, getpid());
    offset += Logger::push_to_buffer_string_safe(buf + offset, "]");
    offset += Logger::push_to_buffer_string_safe(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);

	if (em_ptr)
		em_ptr->kill_emulation();
	delete logger_ptr;

	_exit(signum);
}
