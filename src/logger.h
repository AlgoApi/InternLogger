#ifndef STR_H
#define STR_H
#include <string>
#endif

#ifndef FILE_H
#define FILE_H
#include <cstdio>
#include <filesystem>
#include <fstream>
#endif

#ifndef TIME_H
#define TIME_H
#include <ctime>
#endif

#ifndef EXC_H
#define EXC_H
#include <stdexcept>
#endif

// Listing the logging importance levels
#ifndef LOGGER_H
#define LOGGER_H
enum log_type {
    debug_log_type = 0,
    info_log_type = 1,
    warn_log_type = 2,
    error_log_type = 3,
    critical_log_type = 4,
    _unknown_log_type = -999
};

// Listing of results of logger operations
enum LoggerReturn {
    FILE_OPENED_LOGGER,
    FILE_CLOSED_LOGGER,
    FILE_ALREADY_OPEN_LOGGER,
    FILE_ALREADY_CLOSED_LOGGER,
    LOG_SAVED_LOGGER,
    LOG_FAILED_LOGGER,
    LOG_SKIPPED_LOGGER,
    FILE_CANNOT_OPEN_FOR_WRITING_LOGGER,
    FILE_UNAVAILABLE_LOGGER,
    FILE_INCORRECT_LOGGER,
    OK_LOGGER
};

// Structure of formatted time from strings
struct time_string {
    char hour[3];
    char min[3];
    char sec[3];
};

// здесь использование нескольких точек выхода
// мне показалось более читаемо
/**
 * @brief Convert log_type to string.
 *
 * Convert log_type to string, if not match - UNKNOWN
 *
 * @param[in] mode log_type.
 *
 * @return string log type
 */
const char* _log_type_to_string(log_type mode);

class logger {
    // current log level/logger mode (The importance level)
    log_type mode = info_log_type;

    // path to output file
    std::string path;

    // logger status
    LoggerReturn logger_status = OK_LOGGER;

    // file stream
    std::ofstream file;

   public:
    /**
     * @brief Class logger constructor of a class with 2 arguments.
     *
     * Constructor of a class with path and mode & valdate file path
     *
     * @param[in] path_v path.
     * @param[in] mode_v log_type.
     *
     */
    logger(const std::string& path_v, const log_type mode_v);

    /**
     * @brief Class logger constructor of a class with 1 arguments.
     *
     * Constructor of a class with path & valdate file path
     *
     * @param[in] path_v path.
     *
     */
    explicit logger(const std::string& path_v);

    /**
     * @brief Class logger destructor.
     *
     * Class logger destructor, closing the file correctly
     */
    ~logger();

    /**
     * @brief Setter for logger mode.
     *
     * Sets the mode value if this value is not _unknown_log_type
     *
     * @param[in] mode_v log_type.
     *
     * @return if return _unknown_log_type - value not set, otherwise return new mode
     */
    log_type set_mode(const log_type mode_v);

    /**
     * @brief Getter for logger mode.
     *
     * Get current mode
     *
     * @return current mode
     */
    log_type get_mode() const;

    /**
     * @brief Getter for logger status.
     *
     * Get current status
     *
     * @return current status
     */
    LoggerReturn get_status() const;

    /**
     * @brief Logger run function.
     *
     * Opens the file for writing to the bottom
     *
     * @return file opening status:
     * FILE_ALREADY_OPEN_LOGGER,
     * FILE_CANNOT_OPEN_FOR_WRITING_LOGGER,
     * FILE_OPENED_LOGGER
     */
    LoggerReturn run_logger();

    /**
     * @brief Logger stop function.
     *
     * Close the file
     *
     * @return file closing status:
     * FILE_ALREADY_CLOSED_LOGGER,
     * FILE_CLOSED_LOGGER
     */
    LoggerReturn stop_logger();

    /**
     * @brief Put an entry in a file.
     *
     * Put an entry in a file if the log_type is not less than
     * the default value and file exists and valid
     *
     * @param[in] message message.
     * @param[in] mode_v log_type.
     *
     * @return put entry status:
     * LOG_FAILED_LOGGER,
     * FILE_CLOSED_LOGGER,
     * LOG_SAVED_LOGGER
     */
    LoggerReturn put_log(const std::string& message, const log_type mode_v);
};
#endif