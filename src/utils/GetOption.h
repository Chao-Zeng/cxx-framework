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

private:
    void showVersion(const std::string &programName);

private:
    int m_argc = 0;
    char** m_argv = nullptr;
    bool m_help = false;
    bool m_version = false;
    logger::severity_level m_log_level = logger::debug;
    std::string m_configFile;
};

} // namespace util

#endif // _GET_OPTION_H_

