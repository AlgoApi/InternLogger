#include "logger.h"

// коментарии в header (.h) файле или наведитесь курсором на функцию

const char* _log_type_to_string(log_type mode) {
    switch (mode) {
        case debug_log_type:
            return "DEBUG";
        case info_log_type:
            return "INFO";
        case warn_log_type:
            return "WARN";
        case error_log_type:
            return "ERROR";
        case critical_log_type:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief Getting the current time structure.
 *
 * Getting the current time structure with correct formatting in strings
 *
 * @return struct time_string
 */
const time_string _get_time() {
    std::time_t now_time_t = std::time(nullptr);
    const std::tm* now_tm = std::localtime(&now_time_t);
    struct time_string time;
    if (now_tm->tm_hour < 10) {
        time.hour[0] = '0';
        time.hour[1] = now_tm->tm_hour + '0';
    } else {
        time.hour[0] = now_tm->tm_hour / 10 + '0';
        time.hour[1] = now_tm->tm_hour % 10 + '0';
    }
    if (now_tm->tm_min < 10) {
        time.min[0] = '0';
        time.min[1] = now_tm->tm_min + '0';
    } else {
        time.min[0] = now_tm->tm_min / 10 + '0';
        time.min[1] = now_tm->tm_min % 10 + '0';
    }
    if (now_tm->tm_sec < 10) {
        time.sec[0] = '0';
        time.sec[1] = now_tm->tm_sec + '0';
    } else {
        time.sec[0] = now_tm->tm_sec / 10 + '0';
        time.sec[1] = now_tm->tm_sec % 10 + '0';
    }
    time.hour[2] = '\0';
    time.min[2] = '\0';
    time.sec[2] = '\0';
    return time;
}

/**
 * @brief Validate file path.
 *
 * Validation of the file path and the file itself
 *
 * @param[in] path path.
 *
 * @return file checking status:
 * OK_LOGGER,
 * FILE_INCORRECT_LOGGER,
 * FILE_UNAVAILABLE_LOGGER
 */
LoggerReturn _check_file(const std::string& path) {
    LoggerReturn result = OK_LOGGER;
    if (path.size() < 4 ||
        (path.substr(path.size() - 4) != ".txt" && path.substr(path.size() - 4) != ".log")) {
        result = FILE_INCORRECT_LOGGER;
    }
    std::ifstream test(path);
    if (!test.good()) {
        test.close();
        result = FILE_UNAVAILABLE_LOGGER;
    }
    test.close();
    return result;
}

logger::logger(const std::string& path_v, const log_type mode_v) : mode(mode_v), path(path_v) {
    if (mode == _unknown_log_type) {
        mode = info_log_type;
    }
    logger_status = _check_file(path_v);
}

logger::logger(const std::string& path_v) : path(path_v) { logger_status = _check_file(path_v); }

log_type logger::set_mode(const log_type mode_v) {
    log_type result = _unknown_log_type;
    if (mode_v != _unknown_log_type) {
        mode = mode_v;
        result = mode;
    }
    return result;
}

log_type logger::get_mode() const { return mode; }

LoggerReturn logger::get_status() const { return logger_status; }

LoggerReturn logger::run_logger() {
    LoggerReturn result = FILE_ALREADY_OPEN_LOGGER;
    if (!file.is_open()) {
        file.open(path, std::ios::app);
        if (!file.is_open()) {
            result = FILE_CANNOT_OPEN_FOR_WRITING_LOGGER;
        } else {
            result = FILE_OPENED_LOGGER;
        }
    }
    return result;
}

LoggerReturn logger::stop_logger() {
    LoggerReturn result = FILE_ALREADY_CLOSED_LOGGER;
    if (file.is_open()) {
        file.close();
        result = FILE_CLOSED_LOGGER;
    }
    return result;
}

LoggerReturn logger::put_log(const std::string& message, const log_type mode_v) {
    LoggerReturn result = LOG_SKIPPED_LOGGER;
    bool error = false;
    if (!std::filesystem::exists(path)) {
        file.flush();
        result = LOG_FAILED_LOGGER;
        error = true;
    }
    if (!error && (mode_v >= mode || mode_v == _unknown_log_type)) {
        log_type curr_mode = mode;
        if (mode_v != _unknown_log_type) {
            curr_mode = mode_v;
        }
        if (!file.is_open()) {
            file.flush();
            result = FILE_CLOSED_LOGGER;
            error = true;
        }
        file << "[";
        if (file.fail() || !file.good()) {
            if (!error) {
                file.flush();
                result = LOG_FAILED_LOGGER;
            }
        } else {
            struct time_string time = _get_time();

            file << _log_type_to_string(curr_mode) << "] " << message << " " << time.hour << ":" << time.min
                 << ":" << time.sec << std::endl;
            if (file.fail() || !file.good()) {
                file.flush();
                result = LOG_FAILED_LOGGER;
            } else {
                result = LOG_SAVED_LOGGER;
            }
            file.flush();
        }
    }

    return result;
}

logger::~logger() {
    if (file.is_open()) {
        file.close();
    }
}