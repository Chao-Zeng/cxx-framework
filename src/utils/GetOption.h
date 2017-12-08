#ifndef _GET_OPTION_H_
#define _GET_OPTION_H_

#include <string>

#include "log/log.h"

namespace util
{

class GetOption
{
public:
    GetOption(int argc, char **argv);
    bool parseOption();
    bool isShowHelp() const {return m_help;}
    bool isShowVersion() const {return m_version;}
    bool isDaemon() const {return m_daemon;}
    std::string getConfigFile() const {return m_configFile;}
    bool isSignal() const {return !m_signalName.empty();}
    std::string getSignalName() const {return m_signalName;}
    bool isChangeLogLevel() const {return !m_log_level_str.empty();}
    logger::severity_level getLogLevel() const {return m_log_level;}
    std::string getLogLevelStr() const {return m_log_level_str;}
    void showUsage(const std::string &programName) const;

private:
    int getLogLevel(const std::string &logLevelStr) const;

private:
    int m_argc = 0;
    char** m_argv = nullptr;
    bool m_help = false;
    bool m_version = false;
    bool m_daemon = false;
    std::string m_configFile = "";
    std::string m_signalName = "";

    std::string m_log_level_str = "";
    logger::severity_level m_log_level = logger::info;
};

} // namespace util

#endif // _GET_OPTION_H_

