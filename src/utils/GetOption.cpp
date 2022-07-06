#include "GetOption.h"

#include <getopt.h>
#include <stdio.h>

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
            {"version", no_argument, 0, 'v'},
            {"daemon", no_argument, 0, 'd'},
            {"config", required_argument, 0, 'c'},
            {"log", required_argument , 0, 'l'},
            {"signal", required_argument , 0, 's'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        ret = getopt_long(m_argc, m_argv, "hvdc:l:s:", long_options, &option_index);

        // detect the end of the options
        if(-1 == ret)
        {
            break;
        }

        switch(ret)
        {
            case 0:
                if (long_options[option_index].flag != 0)
                    break;
                printf("option %s", long_options[option_index].name);
                if (optarg)
                {
                    printf("with arg %s", optarg);
                }
                printf("\n");
                break;

            case 'h':
                m_help = true;
                break;

            case 'd':
                m_daemon = true;
                break;

            case 'v':
                m_version = true;
                break;

            case 'c':
                m_configFile = optarg;
                break;

            case 'l':
                if (getLogLevel(optarg) < 0)
                {
                    return false;
                }
                m_log_level_str = optarg;
                m_log_level = static_cast<logger::severity_level>(getLogLevel(optarg));
                break;

            case 's':
                m_signalName = optarg;
                break;

            case '?':
                // getopt_long already printed an error message
                return false;

            default:
                return false;
        }
    }

    return true;
}

void GetOption::showUsage(const std::string &programName) const
{
    printf(
        "Usage: %s [OPTION]...\n"
        "  -h, --help                : show this message\n"
        "  -d, --daemon              : daemon process\n"
        "  -l, --log level           : change log level: trace debug info warn error fatal\n"
        "  -v, --version             : show version info\n"
        "  -c, --config filename     : set configuration file\n"
        "  -s, --signal signal       : send signal to daemon process: stop quit\n"
        , programName.c_str()
        );
}

int GetOption::getLogLevel(const std::string &logLevelStr) const
{
    if (logLevelStr == "trace")
    {
        return logger::trace;
    }
    else if (logLevelStr == "debug")
    {
        return logger::debug;
    }
    else if (logLevelStr == "info")
    {
        return logger::info;
    }
    else if (logLevelStr == "warn")
    {
        return logger::warn;
    }
    else if (logLevelStr == "error")
    {
        return logger::error;
    }
    else if (logLevelStr == "fatal")
    {
        return logger::fatal;
    }
    else
    {
        return -1;
    }
}

} // namespace util

