#ifndef THREAD_H
#define THREAD_H
#include <thread>
#endif

#ifndef QUEUE_H
#define QUEUE_H
#include <condition_variable>
#include <mutex>
#include <queue>
#endif

#ifndef IO_H
#define IO_H
#include <iostream>
#endif

#ifndef SIGINT_H
#define SIGINT_H
#include <atomic>
#include <csignal>
#endif

#ifndef LOGGER_H
#include "logger.h"
#endif

// Queue element
#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H
struct LogEntry {
    std::string type;
    std::string message;
};
#endif

#ifndef MAINFRAME_H
#define MAINFRAME_H
void handle_sigint(int);

/**
 * @brief string to log_type.
 *
 * converting a logging level string to one of the log_type elements.
 *
 * @param[in] level string containing the logging level.
 * @return log_type,
 * @return _unknown_log_type  If no matches are found.
 */
log_type str_to_log_type(const std::string& level);

/**
 * @brief custom std::getline.
 *
 * getting each character and putting it in a string until the character is \\n, \0 or EOF.
 *
 * @param[in] in an open stream for reading.
 * @param[out] out reading result.
 *
 * @return if the stream ended immediately and out is empty - false, otherwise true.
 */
bool s21_getline(std::istream& in, std::string& out);

/**
 * @brief print command & LoggerReturn to std::cout.
 *
 * Convert LoggerReturn to a string and
 * print command first and then this string.
 *
 * @param[in] status LoggerReturn.
 * @param[in] out command.
 */
void print_logger_status(const LoggerReturn status, const std::string& command);

/**
 * @brief compare strings ignoring the time part.
 *
 * Compare strings ignoring the time part. It is assumed that the time starts after the last space
 *
 * @param[in] expected expected string.
 * @param[in] actual actual string.
 *
 * @return true if the strings are equal, otherwise false.
 */
bool compare_lines_ignore_time(const std::string& expected, const std::string& actual);

/**
 * @brief compare files line by line.
 *
 * Compare files line by line and print reading errors or
 * the first occurrence of unequal lines of a file in the format
 * "Line <line_num> not mach:\n
 *     expected: <exp_line>\n
 *     get:  <act_line>\n"
 * and the result of the comparison
 *
 * @param[in] expected_file path to expected file.
 * @param[in] actual_file path to actual file.
 */
void compare_files(const std::string& expected_file, const std::string& actual_file);

/**
 * @brief split line.
 *
 * Split line by delim into the first two parts.
 *
 * @param[in] delim delimiter.
 * @param[out] first_part result of slpliting - first part.
 * @param[out] second_part result of slpliting - second part.
 *
 */
void split_info(std::string& first_part, std::string& second_part, std::string delim,
                const std::string& input);

/**
 * @brief log level setter interface.
 *
 * if the string containing the logging level is
 * recognized, then a new default value is set in the log class
 * and print status.
 *
 * @param[in] log logger.
 * @param[in] log_str level log string.
 *
 */
void mode_setter_interface(logger& log, const std::string& log_str);

/**
 * @brief Separate thread for the operations of the logger.
 *
 * The loop takes a new pair of values from
 * the queue and performs operations (put_log or mode_setter_interface).
 * The loop runs until shutdown is set to true.
 *
 * @param[in] log logger.
 * @param[in] queue queue<LogEntry>.
 * @param[in] mtx provides thread safety.
 * @param[in] cv provides waiting during an empty queue.
 * @param[in] shutdown flag that ensures that the queue is stopped after processing.
 */
void logger_thread(logger& log, std::queue<LogEntry>& queue, std::mutex& mtx, std::condition_variable& cv,
                   const bool& shutdown);

/**
 * @brief Separate thread for the input from console.
 *
 * The loop runs until interrupted true is set.
 * Divides the input into the first two parts,
 * where the first is the logging level or command,
 * and the second is the message. After which it gets into the queue for the logger
 * (Аfter pressing Ctrl + C is necessary to complete the cycle, that is, press Enter).
 *
 * @param[in] queue queue<LogEntry>.
 * @param[in] mtx provides thread safety.
 * @param[in] cv provides wake-up logger_thread after sending data to the queue.
 * @param[in] interrupted Stop flag when pressing Ctrl + C
 */
void input_loop(std::queue<LogEntry>& queue, std::mutex& mtx, std::condition_variable& cv,
                std::atomic<bool>& interrupted);
#endif