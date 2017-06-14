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

        int option_index = 0;

        ret = getopt_long(m_argc, m_argv, "hdivc:", long_options, &option_index);

        //Detect the end of the options
        if(-1 == ret)
        {
            break;
        }

        switch(ret)
        {
            case 0:
                {
                    if(long_options[option_index].flag != 0)
                        break;
                    std::cout << "option " << long_options[option_index].name;
                    if(optarg)
                    {
                        std::cout << "with arg " << optarg;
                    }
                    std::cout << std::endl;
                    break;
                }
            case 'h':
                {
                    m_help = true;
                    break;
                }
            case 'd':
                {
                    m_debug_log = true;
                    break;
                }
            case 'i':
                {
                    m_info_log = true;
                    break;
                }
            case 'v':
                {
                    m_version = true;
                    break;
                }
            case 'c':
                {
                    m_configFile = optarg;
                    break;
                }
            case '?':
                {
                    // getopt_long already printed an error message
                    return false;
                }
            default:
                {
                    return false;
                }
        }
    }

    return true;
}

void GetOption::showUsage(const std::string &programName)
{
    std::cout << "Usage: " << programName << " [OPTION]...\n";
    std::cout << "\t -h, --help \t\t" << "show this message\n";
    std::cout << "\t -d, --debug \t\t" << "change log level to debug\n";
    std::cout << "\t -i, --info \t\t" << "change log level to info\n";
    std::cout << "\t -v, --version \t\t" << "show version info\n";
    std::cout << "\t -c, --config \t\t" << "set config file\n";
}

} // namespace util

