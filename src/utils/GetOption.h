#ifndef _GET_OPTION_H_
#define _GET_OPTION_H_

#include <string>

namespace util
{

class GetOption
{
public:
    GetOption(int argc, char **argv);
    bool parseOption();
    bool isShowHelp() {return m_help;}
    bool isShowVersion() {return m_version;}
    bool isSetDebugLog() {return m_debug_log;}
    bool isSetInfoLog() {return m_info_log;}
    std::string getConfigFile() {return m_configFile;}
    void showUsage(const std::string &programName);

private:
    int m_argc = 0;
    char** m_argv = nullptr;
    bool m_help = false;
    bool m_version = false;
    bool m_debug_log = false;
    bool m_info_log = false;
    std::string m_configFile = "";
};

} // namespace util

#endif // _GET_OPTION_H_

