#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <memory>
#include <iomanip>
#include <chrono>
#include <cstdarg>
#include <cstdio>

namespace ThreadLogger {

class Logger {
public:
    explicit Logger(const std::string& base_name)
    {
        std::ostringstream oss;
        oss << base_name << "_thread_" << std::this_thread::get_id() << ".log";
        file_.open(oss.str(), std::ios::out | std::ios::app);
    }

    ~Logger() {
        if (file_.is_open()) file_.close();
    }

    // 支持类似 printf 的格式化输出
    void log(const std::string& level, const char* fmt, ...) {
        if (!file_.is_open()) return;

        // 时间戳
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&t);
        file_ << "[" << std::put_time(&tm, "%F %T") << "]"
              << "[" << level << "] ";

        // 格式化内容
        va_list args;
        va_start(args, fmt);
        char buffer[1024];
        std::vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        file_ << buffer << '\n';
        file_.flush();
    }

private:
    std::ofstream file_;
};

inline std::mutex g_mutex;
inline std::unordered_map<std::thread::id, std::shared_ptr<Logger>> g_loggers;

// 获取线程对应的 logger（线程安全）
inline std::shared_ptr<Logger> getThreadLogger(const std::string& base_name = "log") {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto tid = std::this_thread::get_id();
    auto it = g_loggers.find(tid);
    if (it == g_loggers.end()) {
        auto logger = std::make_shared<Logger>(base_name);
        g_loggers[tid] = logger;
        return logger;
    }
    return it->second;
}

// 宏封装
#define LOG_INFO(fmt, ...) ThreadLogger::getThreadLogger()->log("INFO", fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) ThreadLogger::getThreadLogger()->log("WARN", fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) ThreadLogger::getThreadLogger()->log("ERROR", fmt, __VA_ARGS__)

} // namespace ThreadLogger
