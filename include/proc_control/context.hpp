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
    PacketsBuffer to_send;
    PacketsBuffer to_receive;
};

/*
 * Awakes emulated process with a given em_proc and lets him run 
 * for specified time.
 * 
 * The timing assumption of this functions are as follows:
 * 
 * - Send SIGCONT to em_proc
 * [negligable amount of time]
 * - em_proc truly starts working
 * [negligable amount of time]
 * - This process gets SIGCHLD signal (informing that em_proc indeed started working)
 * - t1 = gettime()
 * - sleep for ts
 * - send SIGSTOP to em_proc
 * [unspecified amount of time]
 * - em_proc truly stops working
 * [negligable amount of time]
 * - this process gets SIGCHLD signal (informing that em_proc indeed started working)
 * - t2 = gettime()
 * - return t2 - t1 - ts (how much more did the process work then we wanted it to,
 *          we hope this number to be small, but for control we add this).
 * 
 * Waiting for the second SIGCHLD signal is necessary to assure that only one
 * emulated process is running concurrently
 */
struct timespec awake(EMProc& emproc, struct timespec ts, int tun_fd);

#endif // PLAYGROUND_CONTEXT