#include <iostream>
#include "version.h"
#include "utils/GetOption.h"
#include "config/config.h"
#include "log/log.h"

#include <unistd.h>

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

    logger::init_log(config::Config::instance().getLogFile());
    LOG(info, "init log success");
    LOG(info, "test log");
    LOG(error, "error log");
    LOG(fatal, "fatal log");

    sleep(2); // sleep for log thread

    return 0;
}

