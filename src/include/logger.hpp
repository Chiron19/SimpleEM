#pragma once

#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include <string>

#include "utils.hpp"


class Logger {

    FILE* file_ptr;

public:

    Logger(const std::string& file_path);
    ~Logger();

    void log_event(clockid_t clock_type, const char* format, va_list args);
    void log_event(clockid_t clock_type, const char* format, ...);
    void log_event(const char* format, ...);

    /** \brief Appends current time to buffer (async-signal-safe)
     */
    static size_t push_to_buffer_time_safe(char* buf, clockid_t clk_id); 
    static size_t push_to_buffer_string_safe(char* buf, const char* expr);
    static size_t push_to_buffer_int_safe(char* buf, int expr);
    static void dump(const char *buf, size_t len);

    /** \brief Print to STDOUT (acync-signal-safe) 
     */
    static void print_string_safe(const std::string& expr);
    static void print_int_safe(int expr);
    static void print_time_safe(clockid_t clk_id);


private:

    void log_time(clockid_t clock_type); // Logs without endline after

};

/* Gloabal logger */
extern Logger* logger_ptr;


Logger::Logger(const std::string& file_path) {
    file_ptr = fopen(file_path.c_str(), "w");
}

Logger::~Logger() {
    fclose(file_ptr);
}

void Logger::log_event(clockid_t clock_type, const char* format, va_list args) {
    log_time(clock_type);
    vfprintf(file_ptr, format, args);
    fprintf(file_ptr, "\n");
}

void Logger::log_event(clockid_t clock_type, const char* format, ...) {
    va_list args;
    va_start(args, format);

    log_time(clock_type);
    vfprintf(file_ptr, format, args);
    fprintf(file_ptr, "\n");
    
    va_end(args);
}

void Logger::log_event(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_event(CLOCK_MONOTONIC, format, args);
    va_end(args);
}

void Logger::log_time(clockid_t clock_type) {
    struct timespec res;
    clock_gettime(clock_type, &res);

    fprintf(file_ptr,
        "[sec: %06d msec: %03d micsec: %03d]",
        get_sec(res), 
        get_msec(res), 
        get_micsec(res));
}

size_t Logger::push_to_buffer_time_safe(char* buf, clockid_t clk_id) {
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

    offset += push_to_buffer_int_safe(buf + offset, (int)get_sec(res));

    strcpy(buf + offset, msec_indicator);
    offset += strlen(msec_indicator);

    offset += push_to_buffer_int_safe(buf + offset, get_msec(res));

    strcpy(buf + offset, micsec_indicator);
    offset += strlen(micsec_indicator);

    offset += push_to_buffer_int_safe(buf + offset, get_micsec(res));

    strcpy(buf + offset, time_suff_indicator);
    offset += strlen(time_suff_indicator);
    
    return offset;
}

size_t Logger::push_to_buffer_string_safe(char* buf, const char* expr) {
    size_t len = strlen(expr);
    strncpy(buf, expr, len);
    return len;
}

size_t Logger::push_to_buffer_int_safe(char* buf, int expr) {
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

void Logger::print_string_safe(const std::string& expr) {
    write(STDOUT_FILENO, expr.c_str(), expr.size());
}

void Logger::print_int_safe(int expr) {
    char buf[BUF_SIZE];
    size_t len = Logger::push_to_buffer_int_safe(buf, expr);
    write(STDOUT_FILENO, buf, len);
}   

void Logger::print_time_safe(clockid_t clk_id) {
    char buf[BUF_SIZE];
    size_t len = Logger::push_to_buffer_time_safe(buf, clk_id);
    write(STDOUT_FILENO, buf, len);
}

void Logger::dump(const char *buf, size_t len) {
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