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

/** Type of emulator's internal process id  */
typedef int em_id_t; 

/** @brief Controller of a single process inside an emulation
 * 
 * Class responsible for the state and awakening of a single process 
 * in an emulation. It holds the queues of packets that were sent from the
 * process as well as the queue of packets that should be delivered to the
 * process. It keeps track of process virtual clock.
 * All the inside-awakening logic is kept in this class. 
 * 
*/
class EMProc {

public:

    em_id_t em_id; ///< Internal id of emulated process
    int pid; ///< pid of emulated process
    struct timespec virtual_clock; ///< Time that the process was awake
    std::priority_queue<Packet> out_packets; ///< Buffer of packets sent by process
    std::priority_queue<Packet> in_packets; ///< Buffer of packets to be received by process

    /** @brief Class main constructor
     * 
     * Sets process' virtual clock to 0. 
     * 
     * @param em_id Emulator's internal process id of this process
     * @param pid Operating systems pid of process associated with this emulated process
    */
    EMProc(em_id_t em_id, int pid): em_id(em_id), pid(pid), 
                                    virtual_clock({0, 0}) {}

    /** @brief Awake emulated process and let him run for
     *         specified amount of time, intercepting packets sent by it.
     * 
     * Awakes emulated process for amount of time specified by @p ts .
     * All packets sent by this process to addresses in the subnetwork 
     * of TUN interface specified by @p tun_fd will be intercepted and stored
     * in out_packets.
     * 
     * Function sends SIGCONT signal to specified process, then after @p ts time
     * it sends the SIGSTOP signal. During the time that the process is running
     * all packets sent by it will be intercepted. Also, in appropriate times,
     * packets from in_packets will be sent to the process using 
     * the TUN interface with @p tun_fd . virtual_clock is updated 
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
            break; /* Appropriate time run */
        
        /* If anything should be sent to this process in this loop, send it */
        while (to_receive_before(virtual_clock + elapsed_time)) {
            logger_ptr->log_event("Sending something at proc time: %ld", nano_from_ts(virtual_clock + elapsed_time));

            network.send(in_packets.top());

            // printf("[emproc.hpp] In_packets: ------- em_id: %d\n", em_id);
            // Packet packet1 = in_packets.top();
            // packet1.dump();
            // printf("  %s -> %s\n", packet1.get_source_addr().c_str(), packet1.get_dest_addr().c_str()); 
            this->in_packets.pop();
        }

        /* If running process sent anything, save it */
        ssize = network.receive(buf, sizeof(buf));
        if (ssize <= 0) {
            // printf("[emproc.hpp] NONE RECV!\n");
            continue; /* Nothing received*/
        }  

        // printf("[emproc.hpp] Received: %s, size: %ld\n", buf, ssize);
        // dump(buf, ssize);

        Packet packet(buf, ssize, this->virtual_clock + elapsed_time);
        // printf("%s(%d) -> %s(%d)\n", packet.get_source_addr().c_str(), packet.get_source_port(), packet.get_dest_addr().c_str(), packet.get_dest_port());
        // packet.dump();

        if (packet.get_version() != 4)
            continue; /* IP version not supported */
        if (network.get_em_id(packet.get_dest_addr()) < 0)
            continue; /* Target not in simulated network */
        
        /* Running process sent a valid packet to another process in the system */

        logger_ptr->log_event("Process %d sending packet to process %d (%s), packet length: %d",
            em_id, network.get_em_id(packet.get_dest_addr()), packet.get_dest_addr().c_str(), ssize);
        // Here print process!
        // printf("Process %d sending packet to process %d (%s), packet length: %ld\n", em_id, network.get_em_id(packet.get_dest_addr()), packet.get_dest_addr().c_str(), ssize);
        // printf("[emproc.hpp]Buffer: %s\n", packet.get_buffer());

        // process(packet.get_buffer(), packet.get_size());

        // printf("%s(%d) -> %s(%d)\n", packet.get_source_addr().c_str(), packet.get_source_port_tcp(), packet.get_dest_addr().c_str(), packet.get_dest_port_tcp());
        // printf("Em_id: %d, Dest:%s (em_id: %d)\n",em_id, packet.get_dest_addr().c_str(), network.get_em_id(packet.get_dest_addr()));

        if (network.get_em_id(packet.get_dest_addr()) == em_id) {
            /* Packet sent to my own listening socket, receive immidietly */

            // packet.set_dest_addr(network.get_inter_addr());
            // packet.set_source_addr(network.get_addr(em_id));

            /* Modified from UDP to TCP */
            packet.set_dest_addr_tcp(network.get_inter_addr());
            packet.set_source_addr_tcp(network.get_addr(em_id));

            in_packets.push(packet);
        }
        else {
            out_packets.push(packet);
        }
    }


    kill(this->pid, SIGSTOP);
    sigwait(&to_block, &sig);

    this->virtual_clock = this->virtual_clock + ts;
}

bool EMProc::to_receive_before(struct timespec ts) {
    if (in_packets.empty())
        return false;
    return in_packets.top().get_ts() < ts;
}
