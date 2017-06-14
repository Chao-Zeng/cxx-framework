#include <iostream>
#include "utils/GetOption.h"
#include "version.h"

int main(int argc, char* argv[])
{
    util::GetOption getOption = util::GetOption(argc, argv);
    if(!getOption.parseOption())
    {
        getOption.showUsage(argv[0]);
        return -1;
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
    if(!configFile.empty())
    {
        std::cout << "config file: " << configFile << std::endl;
        return 0;
    }
    
    return 0;
}

