#include "loghandler.h"

namespace loghandlerns {

std::string LogHandler::trim(const char str[])
{
    std::string _str(str);
    _str.at(_str.find_last_of("\r\n")) = '\0';

    return _str;
}

void LogHandler::startLog(std::string outputLogFile = "", std::string errorLogFile = "")
{
    if (outputLogFile.empty())
        outputLogFile = DEFAULT_OUTPUT_LOG_FILE;

    if (errorLogFile.empty())
        errorLogFile = DEFAULT_ERROR_LOG_FILE;

    Logger outputLogger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));

    SharedAppenderPtr stdoutAppender(new log4cplus::ConsoleAppender(false, DEFAULT_IMMEDIATE_FLUSH));
    stdoutAppender->setName("stdoutAppender");
    std::auto_ptr<Layout> stdoutLayout = std::auto_ptr<Layout>(new log4cplus::PatternLayout(DEFAULT_LOG_PATTERN_LAYOUT));
    stdoutAppender->setLayout(stdoutLayout);

    SharedAppenderPtr outputFileAppender(new RollingFileAppender(outputLogFile, MAX_LOG_FILE_SIZE, MAX_LOG_BACKUP_INDEX, DEFAULT_IMMEDIATE_FLUSH));
    outputFileAppender->setName("outputFileAppender");
    std::auto_ptr<Layout> outputFileLayout = std::auto_ptr<Layout>(new log4cplus::PatternLayout(DEFAULT_LOG_PATTERN_LAYOUT));
    outputFileAppender->setLayout(outputFileLayout);

    outputLogger.addAppender(stdoutAppender);
    outputLogger.addAppender(outputFileAppender);
    outputLogger.setLogLevel(DEFAULT_OUTPUT_LOG_LEVEL);


    Logger errorLogger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_ERROR_LOGGER));

    SharedAppenderPtr stderrAppender(new log4cplus::ConsoleAppender(true, DEFAULT_IMMEDIATE_FLUSH));
    stderrAppender->setName("stderrAppender");
    std::auto_ptr<Layout> stderrLayout = std::auto_ptr<Layout>(new log4cplus::PatternLayout(DEFAULT_LOG_PATTERN_LAYOUT));
    stderrAppender->setLayout(stderrLayout);

    SharedAppenderPtr errorFileAppender(new RollingFileAppender(errorLogFile, MAX_LOG_FILE_SIZE, MAX_LOG_BACKUP_INDEX, DEFAULT_IMMEDIATE_FLUSH));
    errorFileAppender->setName("errorFileAppender");
    std::auto_ptr<Layout> errorFileLayout = std::auto_ptr<Layout>(new log4cplus::PatternLayout(DEFAULT_LOG_PATTERN_LAYOUT));
    errorFileAppender->setLayout(errorFileLayout);

    errorLogger.addAppender(stderrAppender);
    errorLogger.addAppender(errorFileAppender);
    errorLogger.setLogLevel(DEFAULT_ERROR_LOG_LEVEL);

    LOG4CPLUS_DEBUG(outputLogger, "Log opened");
}

void LogHandler::stopLog()
{
    Logger outputLogger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));

    LOG4CPLUS_DEBUG(outputLogger, "Log closed");

    Logger::shutdown();
}

int LogHandler::getLogLevel()
{
    return Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER)).getLogLevel();
}

void LogHandler::setLogLevel(int logLevel)
{
    Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER)).setLogLevel(logLevel);
}

}
