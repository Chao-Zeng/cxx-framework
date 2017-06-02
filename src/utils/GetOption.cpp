#include "GetOption.h"

#include <getopt.h>

#include <iostream>

namespace util{

GetOption::GetOption(int argc, char **argv)
    : m_argc(argc), m_argv(argv)
{
}

bool GetOption::parseOption()
{
    if(m_argc < 1 || m_argv == nullptr)
    {
        return false;
    }

    int ret;

    while(true)
    {
        static struct option long_options[] =
        {
            {"help", no_argument, 0, 'h'},
            {"debug", no_argument, 0, 'd'},
            {"info", no_argument, 0, 'i'},
            {"version", no_argument, 0, 'v'},
            {"config", required_argument, 0, 'c'},
            {0, 0, 0, 0}
        };
    }

    return true;
}

void GetOption::showVersion(const std::string &programName)
{
    std::cout << "Usage: " << programName << " [OPTION]...\n";
    std::cout << "\t -h, --help \t\t" << "show this message\n";
    std::cout << "\t -d, --debug \t\t" << "change log level to debug\n";
    std::cout << "\t -i, --info \t\t" << "change log level to info\n";
    std::cout << "\t -v, --version \t\t" << "show version info\n";
    std::cout << "\t -c, --config \t\t" << "set config file\n";
}

} // namespace util

