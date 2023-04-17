#pragma once

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "utils.hpp"
#include "network/packet.hpp"
#include "network/packets_buffer.hpp"

#define SIG_LATENCY_UPPERBOUND 50000000 // in nanoseconds

/* 
 * Type of processes ids inside emulator, numbered from 0 up.
 */
typedef int em_id_t; 

/*
 * Information about an emulated process
 */
class EMProc {
public:
    em_id_t em_id;                  ///< Internal id of emulated process
    int pid;                        ///< pid of emulated process
    struct timespec proc_runtime;   ///< Time that the process was awake
    PacketsBuffer out_packets;      ///< Buffer of packets sent by process
    PacketsBuffer in_packets;       ///< Buffer of packets to be received by process

    /** \brief Function to awake emulated process and let him run for
     *         spefified amount of time, intercepting packets sent by it.
     * 
     * Awakes emulated process for amount of time specified by \p ts .
     * All packets sent by this process to addresses in the subnetwork 
     * of TUN interface specified by \p tun_fd will be intercepted and stored
     * in out_packets.
     * 
     * Function sends SIGCONT signal to specified process, then after \p ts time
     * it sends the SIGSTOP signal. During the time that the process is running
     * all packets sent by it will be intercepted. Also, in appropriate times,
     * packets from in_packets will be sent to the process using 
     * the TUN interface with \p tun_fd . proc_runtime is updated 
     * inside this function. It is guaranteed that when the function
     * returns, the specified process has already stopped.
     * 
     * @param ts Time for the process to run
     * @param tun_fd TUN interface file descriptor
     */
    void awake(struct timespec ts, int tun_fd);

};

void sigchld_handler(int signum);

void EMProc::awake(struct timespec ts, int tun_fd) {
    struct timespec start_time, curr_time, elapsed_time;
    ssize_t ssize;
    char buf[MTU];

    signal(SIGCHLD, SIG_IGN); /* For now, no need to care about it */    
    kill(this->pid, SIGCONT);
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
            this->out_packets.add_packet(buf, ssize, elapsed_time);
        }

        /* If anything should be sent to this process in this loop, send it */
        while (!this->in_packets.packets.empty() && 
               is_greater(elapsed_time, this->in_packets.get_first_ts())) {
            /* First packet from to_receive can be sent */

            Packet packet = this->in_packets.pop_first_packet();
            // TODO sending packet.
        }
    }

    signal(SIGCHLD, sigchld_handler);
    kill(this->pid, SIGSTOP);

    struct timespec req = ts_from_nano(SIG_LATENCY_UPPERBOUND), rem;
    if (nanosleep(&req, &rem) == 0) {
        /* Full sleep performed, without signal, error */
        printf("[CONTEXT SWITCHING ERROR]\n");
        exit(1);
    }
    signal(SIGCHLD, SIG_IGN); /* For now, no need to care about it */

    this->proc_runtime = ts_add(this->proc_runtime, ts);
    log_event("Process %d run for %ld nanoseconds.", this->pid, nano_from_ts(ts));
}

void sigchld_handler(int signum) {
	char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf, "[TINYEM][SIGCHLD]");
    offset += push_to_buffer_time(buf + offset, CLOCK_REALTIME);
    offset += push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);
}