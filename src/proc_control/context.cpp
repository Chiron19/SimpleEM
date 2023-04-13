#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "utils.hpp"
#include "proc_control/context.hpp"
#include "network/buffers.hpp"

void sigchld_handler(int signum) {
	char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf, "[TINYEM][SIGCHLD]");
    offset += push_to_buffer_time(buf + offset, CLOCK_REALTIME);
    offset += push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);
}

struct timespec awake(EMProc& emproc, struct timespec ts, int tun_fd) {
    struct timespec start_time, curr_time, elapsed_time;
    ssize_t ssize;
    char buf[MTU];

    signal(SIGCHLD, SIG_IGN); /* For now, no need to care about it */    
    kill(emproc.pid, SIGCONT);

    /* We assume the time waited between sending SIGCONT and actual process 
    continuing to be negligable, so we don't care about SIGCHLD */

    clock_gettime(CLOCK_REALTIME, &start_time);

    while (true) {
        elapsed_time = get_time_since(start_time);

        if (is_greater(elapsed_time, ts))
            break; /* Apropriate time run */
        
        /* If running process sent anything, save it */
        if ((ssize = read(tun_fd, buf, sizeof(buf))) < 0) {
            if (errno == EAGAIN)
                continue; /* Nothing to read */
            else
                panic("read");
        }
        else {
            emproc.to_send.add_packet(buf, ssize, elapsed_time);
        }

        /* If anything should be sent to this process in this loop, send it */
        while (!emproc.to_receive.packets.empty() && 
               is_greater(elapsed_time, emproc.to_receive.get_first_ts())) {
            /* First packet from to_receive can be sent */

            Packet packet = emproc.to_receive.pop_first_packet();
            // TODO sending packet.
        }
    }

    signal(SIGCHLD, sigchld_handler);
    kill(emproc.pid, SIGSTOP);

    struct timespec req = ts_from_nano(SIG_LATENCY_UPPERBOUND), rem;
    if (nanosleep(&req, &rem) == 0) {
        /* Full sleep performed, without signal, error */
        printf("[CONTEXT SWITCHING ERROR]\n");
        exit(1);
    }
    signal(SIGCHLD, SIG_IGN); /* For now, no need to care about it */

    log_event("Process %d run for %ld nanoseconds.", emproc.pid, nano_from_ts(ts));

    /* nanosleep was interrupted by SIGCHLD, rem holds remaining time to sleep,
    so we slept additional req - rem time */
    return ts_subtract(req, rem);
}