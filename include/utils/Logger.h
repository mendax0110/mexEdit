#ifndef MEXEDIT_LOGGER_H
#define MEXEDIT_LOGGER_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <mutex>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cxxabi.h>

#ifdef DEBUG
#undef DEBUG
#endif

#ifdef ERROR
#undef ERROR
#endif
#ifdef INFO
#undef INFO
#endif
#ifdef TRACE
#undef TRACE
#endif

/// @brief Utilities namespace for mexEdit \namespace mexedit::utils
namespace mexedit::utils
{
    /// @brief Log entry container \struct LogContainer
    struct LogContainer
    {
        /// @brief Log levels \enum Level
        enum class Level { DEBUG, INFO, WARNING, ERR, TRACE };
        Level level;
        std::string message;
        size_t lineNumber{};
        std::string fileName;
        std::string timestamp;

        /**
         * @brief Convert log entry to stringb
         * @return A std::string representation of the log entry
         */
        [[nodiscard]] std::string toString() const
        {
            std::ostringstream oss;
            oss << "[" << timestamp << "] ";
            switch(level)
            {
                case Level::DEBUG:   oss << "[DEBUG] "; break;
                case Level::INFO:    oss << "[INFO] "; break;
                case Level::WARNING: oss << "[WARNING] "; break;
                case Level::ERR:     oss << "[ERROR] "; break;
                case Level::TRACE:   oss << "[TRACE] "; break;
            }
            if (!fileName.empty())
            {
                oss << "(" << fileName << ":" << lineNumber << ") ";
            }
            oss << message;
            return oss.str();
        }
    };

    /// @brief Singleton Logger class \class Logger
    class Logger
    {
    public:
        /**
         * @brief Get the singleton instance of Logger
         * @return A reference to the Logger instance
         */
        static Logger& getInstance()
        {
            static Logger instance;
            return instance;
        }

        /**
         * @brief Add a log entry to the logger
         * @param log The LogContainer entry to add
         */
        void addLog(const LogContainer& log)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            logs_.insert({++logCounter_, log});
        }

        /**
         * @brief Write all logs to a file
         */
        void writeLogsToFile()
        {
            std::filesystem::path logDir = "logs";
            if (!std::filesystem::exists(logDir))
            {
                std::filesystem::create_directories(logDir);
            }

            std::string fileName = "system_" + getCurrentTimestamp() + ".log";
            std::filesystem::path filePath = logDir / fileName;

            std::ofstream file(filePath);
            if (!file.is_open())
            {
                std::cerr << "Could not open log file: " << filePath << std::endl;
                return;
            }

            for (const auto& [_, log] : logs_)
            {
                file << log.toString() << std::endl;
            }

            file.close();
        }

        /**
         * @brief Get the current timestamp as a string
         * @return A string representing the current timestamp
         */
        static std::string getCurrentTimestamp()
        {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::ostringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
            return ss.str();
        }

        inline static thread_local std::vector<std::string> callStack_;

        /**
         * @brief Push a function name onto the breadcrumb stack
         * @param func A string representing the function name
         */
        static void pushBreadcrumb(const std::string& func) { callStack_.push_back(func); }

        /**
         * @brief Pop the last function name from the breadcrumb stack
         */
        static void popBreadcrumb() { if (!callStack_.empty()) callStack_.pop_back(); }

        /**
         * @brief Get the breadcrumb prefix for the current call stack
         * @return A string representing the breadcrumb prefix
         */
        static std::string getBreadcrumbPrefix()
        {
            std::ostringstream oss;
            size_t depth = callStack_.size();
            if (depth == 0)
            {
                return "";
            }

            for (size_t i = 0; i < depth - 1; ++i)
            {
                oss << "|  ";
            }

            oss << "|_ ";
            return oss.str();
        }

        /**
         * @brief Create a prefix string based on the call stack depth
         * @param depth The depth of the call stack
         * @return A string representing the prefix
         */
        static std::string makePrefix(size_t depth)
        {
            std::ostringstream oss;
            for (size_t i = 1; i < depth; ++i)
            {
                oss << "|  ";
            }
            if (depth > 0)
            {
                oss << "|_ ";
            }
            return oss.str();
        }

    private:
        std::map<size_t, LogContainer> logs_;
        int logCounter_ = 0;
        std::mutex mutex_;
        Logger() = default;
    };

    /**
     * @brief Demangle a C++ mangled function name
     * @param mangledName The mangled function name
     * @return A std::string representing the demangled function name
     */
    static inline std::string demangle(const char* mangledName)
    {
        int status = 0;
        std::unique_ptr<char[], void(*)(void*)> demangled(
                abi::__cxa_demangle(mangledName, nullptr, nullptr, &status), std::free
        );
        return (status == 0 && demangled) ? demangled.get() : mangledName;
    }

    /// @brief RAII struct for tracing function entry and exit \struct TraceRAII
    struct TraceRAII
    {
        std::string funcName;
        std::string fileName;
        size_t lineNumber;
        size_t stackDepth;

        /**
         * @brief Constructor that logs function entry
         * @param mangledName The mangled function name
         * @param file The source file name
         * @param line The line number in the source file
         */
        TraceRAII(const char* mangledName, const char* file, size_t line)
                : funcName(demangle(mangledName))
                , fileName(file)
                , lineNumber(line)
        {
            Logger::pushBreadcrumb(funcName);
            stackDepth = Logger::callStack_.size();
            Logger::getInstance().addLog(
            {
                LogContainer::Level::TRACE,
                Logger::makePrefix(stackDepth) + "Entered " + funcName,
                lineNumber,
                fileName,
                Logger::getCurrentTimestamp()
            });
        }

        /**
         * @brief Destructor that logs function exit
         */
        ~TraceRAII()
        {
            Logger::getInstance().addLog(
            {
                LogContainer::Level::TRACE,
                Logger::makePrefix(Logger::callStack_.size()) + "Exited " + funcName,
                lineNumber,
                fileName,
                Logger::getCurrentTimestamp()
            });
            Logger::popBreadcrumb();
        }
    };
} // namespace mexedit::utils


#define LOG_IMPL(level_enum, msg)                                              \
    do {                                                                       \
        std::ostringstream _oss;                                               \
        _oss << mexedit::utils::Logger::getInstance().getBreadcrumbPrefix();   \
        _oss << msg;                                                           \
        mexedit::utils::LogContainer _log;                                     \
        _log.level = level_enum;                                               \
        _log.message = _oss.str();                                             \
        _log.lineNumber = __LINE__;                                            \
        _log.fileName = __FILE__;                                              \
        _log.timestamp = mexedit::utils::Logger::getCurrentTimestamp();        \
        mexedit::utils::Logger::getInstance().addLog(_log);                    \
    } while(0)

#define LOG_DEBUG(msg)   LOG_IMPL(mexedit::utils::LogContainer::Level::DEBUG, msg)
#define LOG_INFO(msg)    LOG_IMPL(mexedit::utils::LogContainer::Level::INFO, msg)
#define LOG_WARNING(msg) LOG_IMPL(mexedit::utils::LogContainer::Level::WARNING, msg)
#define LOG_ERR(msg)     LOG_IMPL(mexedit::utils::LogContainer::Level::ERR, msg)
#define LOG_TRACE(msg)   LOG_IMPL(mexedit::utils::LogContainer::Level::TRACE, msg)

#ifdef NDEBUG
    #define TRACE_FUNC
#else
    #define TRACE_FUNC \
        mexedit::utils::TraceRAII _trace_raii(__PRETTY_FUNCTION__, __FILE__, __LINE__);
#endif

#endif // MEXEDIT_LOGGER_H
