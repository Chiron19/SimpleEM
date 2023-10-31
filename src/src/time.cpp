#include <time.h>
#include <errno.h>
#include <stdio.h>

#include "utils.hpp"

void real_sleep(long long nsecs) {
    struct timespec rem, req = ts_from_nano(nsecs);

    while(1) {
        if (nanosleep(&req, &rem) == 0) {
            /* Full sleep performed, return */
            break;
        }
        else {
            /* 'rem' holds remeining required time */
            req = rem;
        }
    }
}

long long nano_from_ts(struct timespec ts) {
    return (long long)SECOND * ts.tv_sec + ts.tv_nsec;
}

struct timespec ts_from_nano(long long nsecs) {
    return (struct timespec){nsecs / SECOND, nsecs % SECOND};
}

struct timespec operator+(const struct timespec& ts1, 
                          const struct timespec& ts2) {
    return ts_from_nano(nano_from_ts(ts1) + nano_from_ts(ts2));
}

struct timespec operator-(const struct timespec& ts1, 
                          const struct timespec& ts2) {
    return ts_from_nano(nano_from_ts(ts1) - nano_from_ts(ts2));
}

struct timespec operator* (double x, const struct timespec& ts) {
    return ts_from_nano(x * nano_from_ts(ts));
}

struct timespec operator* (const struct timespec& ts, double x) {
    return ts_from_nano(x * nano_from_ts(ts));
}

bool operator>(const struct timespec& ts1, 
               const struct timespec& ts2) {
    return nano_from_ts(ts1) > nano_from_ts(ts2);
}

bool operator<(const struct timespec& ts1, 
               const struct timespec& ts2) {
    return nano_from_ts(ts1) < nano_from_ts(ts2);
}

bool check_if_elapsed(struct timespec ts1, struct timespec ts2) {
    struct timespec curr_time;
    clock_gettime(CLOCK_MONOTONIC, &curr_time);
    return curr_time - ts1 > ts2;
}

struct timespec get_time_since(struct timespec ts1) {
    struct timespec curr_time;
    clock_gettime(CLOCK_MONOTONIC, &curr_time);
    return curr_time - ts1;
}

int get_sec(struct timespec t) {
    return t.tv_sec;
}

int get_msec(struct timespec t) {
    return t.tv_nsec / MILLISECOND;
}

int get_micsec(struct timespec t) {
    return t.tv_nsec / MICROSECOND % MICROSECOND;
}

int get_nsec(struct timespec t) {
    return t.tv_nsec / NANOSECOND % MILLISECOND;
}