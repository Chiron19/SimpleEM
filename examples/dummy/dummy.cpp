#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#include "examples/dummy/dummy.hpp"

#include "utils.hpp"

FILE *logging_fptr;

int main() {
    logging_fptr = open_logging((char*)"dummy", true);

    signal(SIGINT, sigint_handler);

    log_event_proc_cpu_time("Start of %d", getpid());

    while(1) {
        for (int i = 0; i < 1e4; ++i) {}

        log_event_proc_cpu_time("Still alive");
    }

    return 0;
}

void sigint_handler(int signum) {
    char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf, "[DUMMY][SIGINT]");
    offset += push_to_buffer_time(buf + offset, CLOCK_PROCESS_CPUTIME_ID);
    offset += push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);

    fclose(logging_fptr);

    _exit(signum);
}