#pragma once

#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include <string>

/*
    time.c
*/

#define NANOSECOND  1
#define MICROSECOND (int)1e3
#define MILLISECOND (int)1e6
#define SECOND      (int)1e9

/*
 * Sets current process to sleep for the time specified in ts. 
 * 
 * This function solves the issues with signal handling,
 * it resumes 'nanosleep' call after signal handling,
 * ensuring sleep of proper time.
 */
void real_sleep(long long nsecs);

long long nano_from_ts(struct timespec ts);
struct timespec ts_from_nano(long long nsecs);
bool check_if_elapsed(struct timespec ts1, struct timespec ts2); // ts2 elapsed from ts1
struct timespec get_time_since(struct timespec ts1);
struct timespec operator+(const struct timespec& ts1, 
                          const struct timespec& ts2);
struct timespec operator-(const struct timespec& ts1, 
                          const struct timespec& ts2);
struct timespec operator* (double x, const struct timespec& ts);
struct timespec operator* (const struct timespec& ts, double x);
bool operator>(const struct timespec& ts1, 
               const struct timespec& ts2);
bool operator<(const struct timespec& ts1, 
               const struct timespec& ts2);

/*
 * If time was in the form of a*sec + b*msec + c*micsec + d*nsec
 * where 0 <= b,c,d <= 999 it returns apropriate value (a,b,c,d)
 */
int get_sec(struct timespec t);
int get_msec(struct timespec t);
int get_micsec(struct timespec t);
int get_nsec(struct timespec t);

#define BUF_SIZE 320

#define panic(_str) do { perror(_str); abort(); } while (0)
