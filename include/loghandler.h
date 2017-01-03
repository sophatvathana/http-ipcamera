#ifndef LOGHANDLER_H 
#define LOGHANDLER_H 

// log definitions and constants

#include <string>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>

#define DEFAULT_OUTPUT_LOGGER "RTSPClientLibOutput"
#define DEFAULT_ERROR_LOGGER "RTSPClientLibError"

#define DEFAULT_STDOUT_APPENDER DEFAULT_OUTPUT_LOGGER "StdOutAppender"
#define DEFAULT_OUTPUT_FILE_APPENDER DEFAULT_OUTPUT_LOGGER "OutputFileAppender"
#define DEFAULT_STDERR_APPENDER DEFAULT_ERROR_LOGGER "StdErrAppender"
#define DEFAULT_ERROR_FILE_APPENDER DEFAULT_ERROR_LOGGER "ErrorFileAppender"

#define DEFAULT_OUTPUT_LOG_LEVEL INFO_LOG_LEVEL
#define DEFAULT_ERROR_LOG_LEVEL WARN_LOG_LEVEL

#define DEFAULT_IMMEDIATE_FLUSH true

#define DEFAULT_LOG_PATTERN_LAYOUT "%D{%Y-%m-%d %H:%M:%S.%q} [%c] [%H] [%t] [%p] %m%n"
#define DEFAULT_OUTPUT_LOG_FILE "strmrecvclient.output.log"
#define DEFAULT_ERROR_LOG_FILE "strmrecvclient.error.log"

#define MAX_LOG_FILE_SIZE 4194304L
#define MAX_LOG_BACKUP_INDEX 4

using namespace log4cplus;

namespace loghandlerns {

class LogHandler {
public:
    static std::string trim(const char str[]);

    static void startLog(std::string outputLogFile, std::string errorLogFile);
    static void stopLog();
    static int getLogLevel();
    static void setLogLevel(int logLevel);
};

}

#endif // LOGHANDLER_HPP
