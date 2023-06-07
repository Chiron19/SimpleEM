#pragma once

#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include <string>

#include "utils.hpp"

/** @brief Utility class for logging in the system
 * 
 * Logger is used by the emulator for logging all recorded events in 
 * a structured manner. 
 */
class Logger {

    FILE* file_ptr; ///< Pointer to FILE object to which to write

public:

    /** @brief Initializes logger and opens specified file
     * 
     * @param file_path Path to file to which to log to
     */
    Logger(const std::string& file_path);
    ~Logger();

    /** @brief Log specified formatted string (printf style) with time of given clock
     * 
     * @param clock_type Type of clock which will be used to log
     * @param format Format string (printf style)
     * @param args Arguments for the format string
     */
    void log_event(clockid_t clock_type, const char* format, va_list args);

    /** @brief Log specified formatted string (printf style) with time of given clock
     * 
     * @param clock_type Type of clock which will be used to log
     * @param format Format string (printf style)
     */
    void log_event(clockid_t clock_type, const char* format, ...);

    /** @brief Log specified formatted string (printf style) with CLOCK_MONOTONIC
     * @param format Format string (printf style)
     */
    void log_event(const char* format, ...);

    /** @brief Print given buffer to stdout in hex and bin
     * 
     * @param buf Buffer to print
     * @param len Length to print (in buffer)
     */
    static void dump(const char *buf, size_t len);

    /* async-signal-safe functions */

    /** @brief Appends current time to buffer (async-signal-safe)
     * 
     * @param buf Buffer to which to push to
     * @param clk_id Clock to be used
     * @return Number of bytes written
     */
    static size_t push_to_buffer_time_safe(char* buf, clockid_t clk_id); 

    /** @brief Appends string to buffer (async-signal-safe)
     * 
     * @param buf Buffer to which to push to
     * @param expr String to be appended
     * @return Number of bytes written
     */
    static size_t push_to_buffer_string_safe(char* buf, const char* expr);

    /** @brief Appends int to buffer (async-signal-safe)
     * 
     * @param buf Buffer to which to push to
     * @param expr Integer to be appended
     * @return Number of bytes written
     */
    static size_t push_to_buffer_int_safe(char* buf, int expr);

    /** @brief Prints string to STDOUT (async-signal-safe)
     * 
     * @param expr String to be printed
     */
    static void print_string_safe(const std::string& expr);

    /** @brief Prints int to STDOUT (async-signal-safe)
     * 
     * @param expr Integer to be printed
     */
    static void print_int_safe(int expr);

    /** @brief Prints time to STDOUT (async-signal-safe)
     * 
     * @param clk_id Clock to be used
     */
    static void print_time_safe(clockid_t clk_id);


private:

    /** @brief Appends time to logging file (without adding endline after)
     * 
     * @param clock_type Clock to be used
     */
    void log_time(clockid_t clock_type);

};

extern Logger* logger_ptr; ///< Pointer to global logger object


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