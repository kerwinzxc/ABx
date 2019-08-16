#pragma once

#include <fstream>
#include <memory>
#include <iostream>
#include <chrono>
#include <ctime>
#include "Utils.h"
#include <stdarg.h>
#include "ConsoleColor.h"

#if !defined(AB_PRETTY_FUNCTION)
#   if defined __GNUC__
#       define AB_PRETTY_FUNCTION __PRETTY_FUNCTION__
#   elif defined _MSC_VER
#       define AB_PRETTY_FUNCTION __FUNCTION__
#   endif
#endif
// All 24h rotate log
#define LOG_ROTATE_INTERVAL (1000 * 60 * 60 * 24)

namespace IO {

/// Logger class with stream interface
class Logger
{
private:
    enum class Mode
    {
        Stream,
        File
    };
    static std::unique_ptr<Logger> instance_;
#if defined(AB_WINDOWS)
    HANDLE hConsole_{ 0 };
    short foregroundDefault_{ 0 };
#endif
    bool nextIsBegin_{ false };
    std::ofstream fstream_;
    std::ostream& stream_;
    Mode mode_;
    int64_t logStart_;
    using endlType = decltype(std::endl<char, std::char_traits<char>>);
public:
    static std::string logDir_;

    explicit Logger(std::ostream& stream = std::cout);
    explicit Logger(const std::string& fileName) :
        fstream_(fileName),
        stream_(fstream_),
        mode_(Mode::File),
        logStart_(Utils::Tick())
    {}
    ~Logger()
    {
        if (mode_ == Mode::File)
        {
            fstream_.flush();
            fstream_.close();
        }
    }

    // Overload for std::endl only:
    Logger& operator << (endlType endl);
    // Some special types
    Logger& operator << (bool value)
    {
        stream_ << (value ? "true" : "false");
        return *this;
    }
    // Everything else
    template <typename T>
    Logger& operator << (const T& data)
    {
        if (nextIsBegin_)
        {
            if (Utils::TimeElapsed(logStart_) > LOG_ROTATE_INTERVAL)
                Logger::Rotate();
            //set time_point to current time
            std::chrono::time_point<std::chrono::system_clock> time_point;
            time_point = std::chrono::system_clock::now();
            std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);
            struct tm* p;
            p = localtime(&ttp);
            char chr[50] = { 0 };
            strftime(chr, 50, "(%g-%m-%d %H:%M:%S)", p);

            stream_ << std::string(chr) << ": " << data;
            nextIsBegin_ = false;
        }
        else
        {
            stream_ << data;
        }
        return *this;
    }

    void AddError(const std::string& msg)
    {
        Error() << msg << std::endl;
    }
    void AddInfo(const std::string& msg)
    {
        Info() << msg << std::endl;
    }
    void AddWarning(const std::string& msg)
    {
        Warning() << msg << std::endl;
    }
#if defined(PROFILING)
    void AddProfile(const std::string& msg)
    {
        Profile() << msg << std::endl;
    }
#endif
    void AddDebug(const std::string& msg)
    {
        Debug() << msg << std::endl;
    }
    Logger& Error();
    Logger& Info();
    Logger& Warning();
#if defined(PROFILING)
    Logger& Profile();
#endif
    Logger& Debug();
    static int PrintF(const char *__restrict __format, ...);

    static void Close()
    {
        Logger::instance_.reset();
    }
    static void Rotate()
    {
        if (!Logger::logDir_.empty())
        {
            Close();
            Logger::Instance();
        }
    }
    static Logger& Instance();
};

}

#define LOG_INFO (IO::Logger::Instance().Info())
#define LOG_WARNING (IO::Logger::Instance().Warning() << AB_PRETTY_FUNCTION << ": ")
#define LOG_ERROR (IO::Logger::Instance().Error() << AB_PRETTY_FUNCTION << ": ")
#if defined(PROFILING)
#   define LOG_PROFILE (IO::Logger::Instance().Profile())
#elif !defined(LOG_PROFILE)
#   define LOG_PROFILE
#endif
#define LOG_DEBUG (IO::Logger::Instance().Debug() << AB_PRETTY_FUNCTION << ": ")
