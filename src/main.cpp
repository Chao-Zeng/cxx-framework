#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#include <string.h>
#include <stdio.h>

#include <fstream>
#include <string>
#include <vector>

#include "version.h"
#include "utils/GetOption.h"
#include "config/config.h"
#include "log/log.h"

#define PROGRAM_NAME "cxx_framework"

typedef struct {
    int signo;
    std::string signame;
    std::string name;
} signal_t;

std::vector<signal_t> g_signals = {
    {SIGQUIT, "SIGQUIT", "quit" }, // force shutdown

    {SIGTERM, "SIGTERM", "stop" } // terminate gracefully
};

static bool createPidFile(const std::string &filename);
static int sendSignalToDaemon(int signo);

int main(int argc, char* argv[])
{
    util::GetOption getOption = util::GetOption(argc, argv);
    if(!getOption.parseOption())
    {
        getOption.showUsage(PROGRAM_NAME);
        return -1;
    }

    if(getOption.isShowHelp())
    {
        getOption.showUsage(PROGRAM_NAME);
        return 0;
    }

    if(getOption.isShowVersion())
    {
        printf("%s version %d.%d.%d \n", PROGRAM_NAME,
                VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        return 0;
    }

    if(getOption.isChangeLogLevel())
    {
        printf("set log level to %s \n", getOption.getLogLevelStr().c_str());
        return 0;
    }

    std::string configFile = getOption.getConfigFile();
    if(configFile.empty())
    {
        printf("use default config file: config/config.info \n");
        configFile = "config/config.info";
    }

    if(!config::Config::instance().load(configFile))
    {
        printf("read config file %s failed \n", configFile.c_str());
        return -1;
    }

    if (getOption.isSignal())
    {
        for (signal_t signal : g_signals)
        {
            if (signal.name == getOption.getSignalName())
            {
                return sendSignalToDaemon(signal.signo);
            }
        }

        printf("unknown signal %s \n", getOption.getSignalName().c_str());
        return -1;
    }

    /*********** start server logic here *************************************/

    if (getOption.isDaemon())
    {
        // don't change working directory and redirects stdin,stdout,stderr to /dev/null
        if (-1 == daemon(1, 0))
        {
            printf("daemonize failed \n");
            return -1;
        }
    }

    // block signal for all threads
    sigset_t set;
    sigfillset(&set);

    // if SIGBUS, SIGFPE, SIGILL, or SIGSEGV are generated while they are blocked,
    // the result is undefined
    sigdelset(&set, SIGILL);
    sigdelset(&set, SIGFPE);
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGBUS);

    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)
    {
        printf("pthread_sigmask() failed \n");
        return -1;
    }

    logger::init_log(config::Config::instance().getLogFile());
    LOG_INFO("init log success");

    if (!createPidFile(config::Config::instance().getPidFile()))
    {
        LOG_ERROR("create pid file %s failed\n", config::Config::instance().getPidFile().c_str());
        return -1;
    }

    // other logic
    LOG_INFO("test log");
    LOG_ERROR("error log");
    LOG_FATAL("fatal log");

    // start server ok
    LOG_INFO("server start");
    logger::close_console_log();

    // main thread wait for signal
    bool terminate = false;
    for (;;)
    {
        if (terminate)
        {
            break;
        }

        // wait for signal: SIGQUIT SIGTERM
        int signo;
        sigemptyset(&set);
        sigaddset(&set, SIGQUIT);
        sigaddset(&set, SIGTERM);
        if (!getOption.isDaemon())
        {
            sigaddset(&set, SIGINT);
        }
        int err = sigwait(&set, &signo);
        if (err != 0)
        {
            LOG_FATAL("sigwait failed, %d %s", err, strerror(err));
            return -1;
        }

        switch (signo)
        {
            case SIGQUIT:
                LOG_INFO("received %s(%d) signal, force shutdown", strsignal(signo), signo);
                sleep(1);
                return 0;

            case SIGINT:
            case SIGTERM:
                LOG_INFO("received %s(%d) signal, terminate gracefully", strsignal(signo), signo);
                terminate = true;
                break;

            default:
                LOG_ERROR("unexpected signal %s(%d)", strsignal(signo), signo);
                break;
        }
    }

    // program exit cleanup

    remove(config::Config::instance().getPidFile().c_str());

    return 0;
}

bool createPidFile(const std::string &filename)
{
    std::ofstream pidFile(filename);
    if (!pidFile.good())
    {
        LOG_ERROR("open file %s failed, %s\n",
            filename.c_str(), strerror(errno));
        return false;
    }
    pidFile << getpid();
    pidFile.close();
    return true;
}

int sendSignalToDaemon(int signo)
{
    std::ifstream pidFile(config::Config::instance().getPidFile());
    if (!pidFile.good())
    {
        printf("open file %s failed, %s\n",
            config::Config::instance().getPidFile().c_str(), strerror(errno));
        return -1;
    }

    int pid = 0;
    pidFile >> pid;

    if (kill(pid, signo) != -1)
    {
        return 0;
    }

    printf("send %s(%d) signal to process %d failed \n",
            strsignal(signo), signo, pid);
    return -1;
}
