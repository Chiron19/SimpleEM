#ifndef SIMPLEEM_EMPROC
#define SIMPLEEM_EMPROC

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include <queue>

#include "utils.hpp"
#include "network/packet.hpp"

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

    em_id_t em_id;                              ///< Internal id of emulated process
    int pid;                                    ///< pid of emulated process
    struct timespec proc_runtime;               ///< Time that the process was awake
    std::priority_queue<Packet> out_packets;    ///< Buffer of packets sent by process
    std::priority_queue<Packet> in_packets;     ///< Buffer of packets to be received by process

    EMProc(em_id_t em_id, int pid): em_id(em_id), pid(pid), 
                                    proc_runtime({0, 0}) {}

    /** \brief Awake emulated process and let him run for
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

private:

    /** \brief Check if there is any packet that this process should receive
     *         before the specified timestamp.  
     * 
     * Checks if the first packet in in_packets buffer exists and has 
     * timestamp lower then the one specified by \p ts
     * 
     * @param ts Time for which to check
     * @return If such packet exists
     */
    bool to_receive_before(struct timespec ts);

    /** \brief Receive first packet from in_packets
     */
    void receive_first_packet(int tun_fd);

};

void sigchld_handler(int signum);

void EMProc::awake(struct timespec ts, int tun_fd) {
    struct timespec start_time, elapsed_time;
    ssize_t ssize;
    char buf[MTU];

    int sig;
    sigset_t to_block;
    sigemptyset(&to_block);
    sigaddset(&to_block, SIGCHLD);
    sigprocmask(SIG_BLOCK, &to_block, nullptr);

    kill(this->pid, SIGCONT);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    sigwait(&to_block, &sig); 

    while (true) {
        elapsed_time = get_time_since(start_time);

        if (elapsed_time > ts)
            break; /* Apropriate time run */
        
        /* If running process sent anything, save it */
        if ((ssize = read(tun_fd, buf, sizeof(buf))) < 0 && errno != EAGAIN)
            panic("read");
        else if (ssize >= 0)
            this->out_packets.emplace(buf, ssize, 
                                      this->proc_runtime + elapsed_time);
                                      
        /* If anything should be sent to this process in this loop, send it */
        while (to_receive_before(proc_runtime + elapsed_time)) {
            receive_first_packet(tun_fd);
            this->in_packets.pop();
        }
    }

    kill(this->pid, SIGSTOP);
    sigwait(&to_block, &sig);

    this->proc_runtime = this->proc_runtime + ts;
    log_event("Process %d run for %ld nanoseconds.", this->pid, nano_from_ts(ts));
}

bool EMProc::to_receive_before(struct timespec ts) {
    if (in_packets.empty())
        return false;
    return in_packets.top().ts < ts;
}

void EMProc::receive_first_packet(int tun_fd) {
    // TODO   
}

void sigchld_handler(int signum) {
	char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf, "[TINYEM][SIGCHLD]");
    offset += push_to_buffer_time(buf + offset, CLOCK_MONOTONIC);
    offset += push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);
}

#endif // SIMPLEEM_EMPROC