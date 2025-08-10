#include "mainframe.h"

// я не нашёл решения перехвата ctrl+c без глобальных перменных, которое я бы мог понять
// иначе утечки при нажатии ctrl+c и неверно закрытый дескриптор файла
std::atomic<bool> interrupted(false);

void handle_sigint(int) { interrupted = true; }

// коментарии в header (.h) файле или наведитесь курсором на функцию

log_type str_to_log_type(const std::string& level) {
    log_type result = _unknown_log_type;
    if (level == "debug") {
        result = debug_log_type;
    }
    if (level == "info") {
        result = info_log_type;
    }
    if (level == "warn") {
        result = warn_log_type;
    }
    if (level == "error") {
        result = error_log_type;
    }
    if (level == "critical") {
        result = critical_log_type;
    }

    return result;
}

bool s21_getline(std::istream& in, std::string& out) {
    out.clear();
    while (true) {
        int c = in.get();
        if (c == EOF || c == '\n' || c == '\0') {
            return !out.empty();
        }

        char ch = static_cast<char>(c);

        out.push_back(ch);
    }
}

void print_logger_status(const LoggerReturn status, const std::string& command) {
    switch (status) {
        case OK_LOGGER:
            std::cout << command << "\033[32m!!!OK_LOGGER!!!\033[0m";
            break;
        case FILE_OPENED_LOGGER:
            std::cout << command << "\033[32m!!FILE_OPENED!!\033[0m";
            break;
        case FILE_ALREADY_OPEN_LOGGER:
            std::cout << command << "\033[33mFILE_ALREADY_OPEN\033[0m";
            break;
        case FILE_CLOSED_LOGGER:
            std::cout << command << "\033[32m!!FILE_CLOSED!!\033[0m";
            break;
        case FILE_ALREADY_CLOSED_LOGGER:
            std::cout << command << "\033[33mFILE_ALREADY_CLOSED\033[0m";
            break;
        case LOG_FAILED_LOGGER:
            std::cout << command << "\033[31mLOG_FAILED\033[0m";
            break;
        case LOG_SAVED_LOGGER:
            std::cout << command << "\033[32m!!!LOG_SAVED!!!\033[0m";
            break;
        case LOG_SKIPPED_LOGGER:
            std::cout << command << "\033[33mLOG_SKIPPED\033[0m";
            break;
        case FILE_CANNOT_OPEN_FOR_WRITING_LOGGER:
            std::cout << command << "\033[31mFILE_CANNOT_OPEN_FOR_WRITING\033[0m";
            break;
        case FILE_UNAVAILABLE_LOGGER:
            std::cout << command << "\033[31mFILE_UNAVAILABLE\033[0m";
            break;
        case FILE_INCORRECT_LOGGER:
            std::cout << command << "\033[31mFILE_INCORRECT\033[0m";
            break;
    }
    std::cout << std::endl;
}

bool compare_lines_ignore_time(const std::string& expected, const std::string& actual) {
    size_t pos_act = actual.find_last_of(' ');

    std::string trimmed_act = (pos_act == std::string::npos) ? actual : actual.substr(0, pos_act);

    return expected == trimmed_act;
}

void compare_files(const std::string& expected_file, const std::string& actual_file) {
    std::ifstream exp_fs(expected_file);
    std::ifstream act_fs(actual_file);
    bool ok = true;

    if (!exp_fs.is_open() || !exp_fs.good() || exp_fs.fail()) {
        std::cout << "Expected file not valid" << std::endl;
        ok = false;
    }
    if (!act_fs.is_open() || !act_fs.good() || act_fs.fail()) {
        std::cout << "Output file not valid" << std::endl;
        ok = false;
    }

    std::string exp_line, act_line;
    int line_num = 0;
    bool exp_ok = s21_getline(exp_fs, exp_line);
    bool act_ok = s21_getline(act_fs, act_line);

    while (ok && (exp_ok || act_ok)) {
        line_num++;
        if (exp_ok != act_ok) {
            std::cout << "Files not different" << std::endl;
            ok = false;
        }

        if (!compare_lines_ignore_time(exp_line, act_line)) {
            std::cout << "Line " << line_num << " not match:\n"
                      << "  expected: " << exp_line << "\n"
                      << "  get: " << act_line << std::endl;
            ok = false;
        }

        exp_ok = s21_getline(exp_fs, exp_line);
        act_ok = s21_getline(act_fs, act_line);
    }

    if (act_fs.is_open()){
        act_fs.close();
    }

    if (exp_fs.is_open()){
        exp_fs.close();
    }

    if (ok) {
        std::cout << "\033[32mTEST PASSED!\033[0m" << std::endl;
    } else {
        std::cout << "\033[31mTEST FAILED!\033[0m" << std::endl;
    }
}

void split_info(std::string& first_part, std::string& second_part, std::string delim,
                const std::string& input) {
    first_part.clear();
    second_part.clear();
    size_t delimiter_pos = input.find(delim);
    if (delimiter_pos != std::string::npos) {
        first_part = input.substr(0, delimiter_pos);
        second_part = input.substr(delimiter_pos + 1);
    } else {
        first_part = "";
        second_part = input;
    }
}

void mode_setter_interface(logger& log, const std::string& log_str) {
    if (log.set_mode(str_to_log_type(std::ref(log_str))) == _unknown_log_type) {
        std::cout << "Log level is not recognized, the changes are not applied" << std::endl;
    } else {
        std::cout << "Now default: " << _log_type_to_string(log.get_mode()) << std::endl;
    }
}

void logger_thread(logger& log, std::queue<LogEntry>& queue, std::mutex& mtx, std::condition_variable& cv,
                   const bool& shutdown) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] { return !queue.empty() || shutdown; });

        if (shutdown && queue.empty()) break;

        while (!queue.empty()) {
            auto entry = queue.front();
            queue.pop();
            lock.unlock();

            log_type format_type = str_to_log_type(entry.type);

            if (entry.type == "$set_default") {
                mode_setter_interface(std::ref(log), std::ref(entry.message));
            }
            if (entry.type != "$set_default" && format_type == _unknown_log_type) {
                std::cout << "\033[33mUnknown log type, default value is used\033[0m\n";
            }
            if (entry.type != "$set_default") {
                print_logger_status(log.put_log(entry.message, format_type), "put_log: ");
            }

            lock.lock();
        }
    }
}

void input_loop(std::queue<LogEntry>& queue, std::mutex& mtx, std::condition_variable& cv,
                std::atomic<bool>& interrupted) {
    while (!interrupted) {
        std::string input;
        if (s21_getline(std::cin, input) == false || input == "exit") break;

        std::string type_part;
        std::string message_part;
        split_info(std::ref(type_part), std::ref(message_part), ":", input);

        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(LogEntry{type_part, message_part});
        }

        cv.notify_one();
    }
}

/**
 * @brief MAIN FRAME.
 *
 * Guaranteeing minimum 2 console arguments, handling cases:
 * 
 * - incorrect logging level string
 * 
 * - incorrect path to the file or logging file itself.
 * 
 * Initializing the logger, queue, and auxiliary objects.
 * 
 * All paths must be specified relative to the program launch directory.
 * @note When defining TEST_H, testing of the expected (3 console argument)
 * and current output is started.
 *
 * If you want to try the test, set the redirect "<" after the console arguments
 * to the path to the console input: "../../materials/test_input.txt".
 * 
 * Try it: in build/bin directory run this command(bash): 
 * ./main ../../materials/test_output.txt info 
 * ../../materials/test_expected.txt < ../../materials/test_input.txt
 * make sure materials/test_output.txt is empty
 * 
 *
 * @param[in] argc count of console arguments.
 * @param[in] argv array of string console arguments. 
 * argv[1] - path to log file, argv[2] - log level/type, 
 * if defined TEST_H - argv[3] - expected file path
 *
 * @return the result of the entire program
 */
int main(const int argc, const char* argv[]) {
#ifdef TEST_H
    if (argc < 4) {
        std::cout << "Too few arguments";
        return -1;
    }
#else
    if (argc < 3) {
        std::cout << "Too few arguments";
        return -1;
    }
#endif
    log_type user_log_type = str_to_log_type(argv[2]);
    if (user_log_type == _unknown_log_type) {
        std::cout << "\033[33mUnknown log type, default - info is used\033[0m\n";
    }

    logger log(argv[1], user_log_type);
    LoggerReturn satus = log.get_status();
    print_logger_status(satus, "logger_init: ");
    if (satus == FILE_INCORRECT_LOGGER || satus == FILE_UNAVAILABLE_LOGGER) {
        return -1;
    }

    satus = log.run_logger();
    print_logger_status(satus, "run_logger: ");
    if (satus == FILE_CANNOT_OPEN_FOR_WRITING_LOGGER) {
        return -1;
    }

    std::signal(SIGINT, handle_sigint);
    std::queue<LogEntry> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool shutdown = false;

    std::thread log_thread(logger_thread, std::ref(log), std::ref(queue), std::ref(mtx), std::ref(cv),
                           std::ref(shutdown));
    std::cout << "format: log_level:message\n\n";
    input_loop(queue, mtx, cv, interrupted);

    {
        std::lock_guard<std::mutex> lock(mtx);
        shutdown = true;
    }

    cv.notify_one();
    log_thread.join();
    print_logger_status(log.stop_logger(), "stop_logger: ");

#ifdef TEST_H
    compare_files(argv[3], argv[1]);
#endif

    return 0;
}
