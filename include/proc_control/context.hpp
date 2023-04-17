#ifndef PLAYGROUND_CONTEXT
#define PLAYGROUND_CONTEXT

#include <time.h>

#include "utils.hpp"
#include "network/buffers.hpp"

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
    em_id_t em_id;
    int pid;
    struct timespec proc_runtime;   /* Time that the process was active */
    PacketsBuffer out_packets;
    PacketsBuffer in_packets;
};

/** \brief Function to awake chosen emulated process and let him run for
 *         spefified amount of time, intercepting packets sent by it.
 * 
 * Awakes emulated process specified by \p emproc for amount of time
 * specified by \p ts . All packets sent by this process to addresses in
 * TUN subnetwork will be intercepted and stored in 
 * the \p emproc ->out_packets.
 * 
 * Function sends SIGCONT signal to specified process, then after \p ts time
 * it sends the SIGSTOP signal. During the time that the process is running
 * all packets sent by it will be intercepted. Also, in appropriate times,
 * packets from \p emproc ->in_packets will be sent to the process using 
 * the TUN interface with \p tun_fd . It is guaranteed that when the function
 * returns, the specified process has already stopped.
 * 
 * @param emproc Descriptor of emulated process to be awaken
 * @param ts Time for the process to run
 * @param tun_fd TUN interface file descriptor
 */
void awake(EMProc& emproc, struct timespec ts, int tun_fd);

#endif // PLAYGROUND_CONTEXT