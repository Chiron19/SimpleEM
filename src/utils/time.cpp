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
            printf("RESUMING SLEEP\n");
            /* 'rem' holds remeining required time */
            req = rem;
        }
    }
}

void full_sleep(struct timespec ts) {
    struct timespec rem, req = ts;

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

bool is_greater(struct timespec ts1, struct timespec ts2) {
    return nano_from_ts(ts1) > nano_from_ts(ts2);
}

struct timespec ts_subtract(struct timespec ts1, struct timespec ts2) {
    return ts_from_nano(nano_from_ts(ts1) - nano_from_ts(ts2));
}

bool check_if_elapsed(struct timespec ts1, struct timespec ts2) {
    struct timespec curr_time;
    clock_gettime(CLOCK_REALTIME, &curr_time);
    return is_greater(ts_subtract(curr_time, ts1), ts2);
}

struct timespec get_time_since(struct timespec ts1) {
    struct timespec curr_time;
    clock_gettime(CLOCK_REALTIME, &curr_time);
    return ts_subtract(curr_time, ts1);
}

long get_sec(struct timespec t) {
    return t.tv_sec;
}

long get_msec(struct timespec t) {
    return t.tv_nsec / MILLISECOND;
}

long get_micsec(struct timespec t) {
    return t.tv_nsec / MICROSECOND % MICROSECOND;
}

long get_nsec(struct timespec t) {
    return t.tv_nsec / NANOSECOND % MILLISECOND;
}