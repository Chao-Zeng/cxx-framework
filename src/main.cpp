#include <unistd.h> // daemon

#include <iostream>

#include "version.h"
#include "utils/GetOption.h"
#include "config/config.h"
#include "log/log.h"


int main(int argc, char* argv[])
{
    util::GetOption getOption = util::GetOption(argc, argv);
    if(!getOption.parseOption())
    {
        getOption.showUsage(argv[0]);
        return 1;
    }

    if(getOption.isShowHelp())
    {
        getOption.showUsage(argv[0]);
        return 0;
    }

    if(getOption.isShowVersion())
    {
        std::cout << argv[0] << " version " << VERSION_MAJOR << ".";
        std::cout << VERSION_MINOR << "." << VERSION_PATCH << std::endl;
        return 0;
    }

    if(getOption.isSetDebugLog())
    {
        std::cout << "set log level to debug\n";
        return 0;
    }

    if(getOption.isSetInfoLog())
    {
        std::cout << "set log level to info\n";
        return 0;
    }

    std::string configFile = getOption.getConfigFile();
    std::string defaultConfigFile = "./config/config.info";
    if(configFile.empty())
    {
        std::cout << "use default config file: " << defaultConfigFile << std::endl;
        configFile = defaultConfigFile;
    }

    if(!config::Config::instance().load(configFile))
    {
        std::cout << "read config file " << configFile << " failed\n";
        return 1;
    }

    //don't change working directory and redirects stdin,stdout,stderr to /dev/null
    if(-1 == daemon(1,0))
    {
        std::cout << "daemonize failed\n";
        return 1;
    }

    logger::init_log(config::Config::instance().getLogFile());
    LOG(info, "daemonized");
    LOG(info, "init log success");

    LOG(info, "test log");
    LOG(error, "error log");
    LOG(fatal, "fatal log");

    sleep(500);

    return 0;
}

