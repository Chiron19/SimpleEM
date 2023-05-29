#pragma once

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include <queue>

#include "proc_frame.hpp"
#include "logger.hpp"

#include "utils.hpp"
#include "network/packet.hpp"
#include "network/network.hpp"

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
     * @param network Network on which the simulator is operating
     */
    void awake(struct timespec ts, const Network& network);

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

};

void EMProc::awake(struct timespec ts, const Network& network) {
    // logger_ptr->log_event("Process %d awaken, in_packets: %d, out_packets: %d", 
    //     em_id, in_packets.size(), out_packets.size());
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
        
        /* If anything should be sent to this process in this loop, send it */
        while (to_receive_before(proc_runtime + elapsed_time)) {
            logger_ptr->log_event("Sending something at proc time: %ld", nano_from_ts(proc_runtime + elapsed_time));
            network.send(in_packets.top());
            this->in_packets.pop();
        }

        /* If running process sent anything, save it */
        ssize = network.receive(buf, sizeof(buf));
        if (ssize <= 0)
            continue; /* Nothing received*/

        Packet packet(buf, ssize, this->proc_runtime + elapsed_time);
        if (packet.get_version() != 4)
            continue; /* IP version not supported */
        if (network.get_em_id(packet.get_dest_addr()) < 0)
            continue; /* Target not in simulated network */
        
        /* Running process sent a valid packet to another process in the system */

        logger_ptr->log_event("Process %d sending packet to process %d (%s), packet length: %d",
            em_id, network.get_em_id(packet.get_dest_addr()), packet.get_dest_addr().c_str(), ssize);
        // process(packet.get_buffer(), packet.get_size());

        if (network.get_em_id(packet.get_dest_addr()) == em_id) {
            /* Packet sent to my own listening socket, receive immidietly */
            packet.set_dest_addr(network.get_inter_addr());
            packet.set_source_addr(network.get_addr(em_id));

            in_packets.push(packet);
        }
        else {
            out_packets.push(packet);
        }
    }


    kill(this->pid, SIGSTOP);
    sigwait(&to_block, &sig);

    this->proc_runtime = this->proc_runtime + ts;
    // logger_ptr->log_event("Process %d run for %ld nanoseconds, in_packets: %d, out_packets: %d", 
    //     em_id, nano_from_ts(ts), in_packets.size(), out_packets.size());
}

bool EMProc::to_receive_before(struct timespec ts) {
    if (in_packets.empty())
        return false;
    return in_packets.top().get_ts() < ts;
}
