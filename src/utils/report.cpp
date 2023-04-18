#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils.hpp"

size_t push_to_buffer_time(char* buf, clockid_t clk_id) {
    size_t offset = 0;
    struct timespec res;
    const char 
        time_pref_indicator[] =     "[", 
        sec_indicator[] =           "sec: ", 
        msec_indicator[] =          " msec: ", 
        micsec_indicator[] =        " micsec: ",
        time_suff_indicator[] =     "]"; 

    clock_gettime(clk_id, &res);
    
    strcpy(buf, time_pref_indicator);
    offset += strlen(time_pref_indicator);

    strcpy(buf + offset, sec_indicator);
    offset += strlen(sec_indicator);

    offset += push_to_buffer_int(buf + offset, (int)get_sec(res));

    strcpy(buf + offset, msec_indicator);
    offset += strlen(msec_indicator);

    offset += push_to_buffer_int(buf + offset, get_msec(res));

    strcpy(buf + offset, micsec_indicator);
    offset += strlen(micsec_indicator);

    offset += push_to_buffer_int(buf + offset, get_micsec(res));

    strcpy(buf + offset, time_suff_indicator);
    offset += strlen(time_suff_indicator);
    
    return offset;
}

size_t push_to_buffer_string(char* buf, const char* expr) {
    size_t len = strlen(expr);
    strncpy(buf, expr, len);
    return len;
}

size_t push_to_buffer_int(char* buf, int expr) {
    if (expr == 0) {
        buf[0] = '0';
        return 1;
    }

    size_t offset = 0;

    while (expr) {
        buf[offset] = '0' + (expr % 10);
        expr /= 10;
        offset++;
    }

    /* reverse part of the buffer */
    for (size_t i = 0; i < offset / 2; ++i) {
        char helpy = buf[offset - i - 1];
        buf[offset - i - 1] = buf[i];
        buf[i] = helpy;
    }

    return offset;
}

FILE* open_logging(char* prog_name, bool pid_in_name) {
    char buf[BUF_SIZE],
        log_file_pref[]     = "logging_",
        pid_indicator[]     = "_pid_",
        log_file_suf[]      = ".txt";
    size_t offset = 0;

    offset += push_to_buffer_string(buf, log_file_pref);
    offset += push_to_buffer_string(buf + offset, prog_name);
    offset += push_to_buffer_string(buf + offset, pid_indicator);
    if (pid_in_name)
        offset += push_to_buffer_int(buf + offset, getpid());
    offset += push_to_buffer_string(buf + offset, log_file_suf);
    buf[offset] = '\0';

    printf("%s\n", buf);

    return fopen(buf, "w");
}

void log_event(const char* format, ...) {
    FILE *fptr = logging_fptr;

    va_list args;
    va_start (args, format);

    char buf[BUF_SIZE];
    buf[push_to_buffer_time(buf, CLOCK_REALTIME)] = '\0';
    fprintf(fptr, "%s", buf);
    fprintf(fptr, "\t\t");
    vfprintf(fptr, format, args);
    fprintf(fptr, "\n");
    
    va_end(args);
}

void log_event_proc_cpu_time(const char* format, ...) {
    FILE *fptr = logging_fptr;

    va_list args;
    va_start (args, format);

    char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf, "[CPU TIME]");
    offset += push_to_buffer_time(buf + offset, CLOCK_PROCESS_CPUTIME_ID);
    buf[offset] = '\0';
    fprintf(fptr, "%s", buf);
    fprintf(fptr, "\t\t");
    vfprintf(fptr, format, args);
    fprintf(fptr, "\n");

    va_end(args);
}

void dump(const char *buf, size_t len) {
	size_t i, j;

	for (i = 0; i < len; i++) {
		if ((i % 8) == 0)
			printf("%04hx  ", (uint16_t) i);

		printf("%02hhx", buf[i]);
		if ((i % 8) == 3) {
			printf("  ");
		} else if ((i % 8) == 7) {
			printf("  ");
			for (j = i - 7; j <= i; j++)
				if ((buf[j] < 32) || (buf[j] > 126))
					printf(".");
				else
					printf("%c", buf[j]);
			printf("\n");
		} else {
			printf(" ");
		}
	}

	if ((i % 8) != 0) {
		for (j = i % 8; j < 8; j++) {
			printf("  ");
			if (j == 3)
				printf("  ");
			else
				printf(" ");
		}
		printf(" ");
		for (j = i - (i % 8); j < i; j++)
			if ((buf[j] < 32) || (buf[j] > 126))
				printf(".");
			else
				printf("%c", buf[j]);
		printf("\n");
	}
}