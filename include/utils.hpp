#ifndef PLAYGROUND_SLEEP
#define PLAYGROUND_SLEEP

#include <stdio.h>
#include <time.h>
#include <stdbool.h>

/*
    time.c
*/

#define NANOSECOND  1
#define MICROSECOND 1000
#define MILLISECOND 1000000
#define SECOND      1000000000

/*
 * Sets current process to sleep for the time specified in ts. 
 * 
 * This function solves the issues with signal handling,
 * it resumes 'nanosleep' call after signal handling,
 * ensuring sleep of proper time.
 */
void real_sleep(long long nsecs);
void full_sleep(struct timespec ts);

long long nano_from_ts(struct timespec ts);
struct timespec ts_from_nano(long long nsecs);
struct timespec ts_subtract(struct timespec ts1, struct timespec ts2);
bool is_greater(struct timespec ts1, struct timespec ts2); // ts1 > ts2
bool check_if_elapsed(struct timespec ts1, struct timespec ts2); // ts2 elapsed from ts1
struct timespec get_time_since(struct timespec ts1);

/*
 * If time was in the form of a*sec + b*msec + c*micsec + d*nsec
 * where 0 <= b,c,d <= 999 it returns apropriate value (a,b,c,d)
 */
long get_sec(struct timespec t);
long get_msec(struct timespec t);
long get_micsec(struct timespec t);
long get_nsec(struct timespec t);

/* 
    report.c
*/

extern FILE *logging_fptr;

#define BUF_SIZE 160

/*
 * Functions needed for async-signal-safe printing.
 * They do not call functions from printf or other not async-signal-fase functions.
 * 
 * Assume that the buffer has enough space, otherwise undefined behavior. 
 * Return number of chars written to buffer.
 */
size_t push_to_buffer_string(char* buf, const char* expr);
size_t push_to_buffer_int(char* buf, int expr);
size_t push_to_buffer_time(char* buf, clockid_t clk_id);

FILE* open_logging(char* prog_name, bool pid_in_name);
void log_event(const char* format, ...);
void log_event_proc_cpu_time(const char* format, ...);

#define panic(_str) do { perror(_str); abort(); } while (0)

#define RET_UNKNOWN 0
#define RET_IPV4_UDP 1
#define RET_IPV4_TCP 2
#define RET_IPV4_ 3
#define RET_IPV6 11

void dump(const char *buf, size_t len);

#endif // PLAYGROUND_SLEEP