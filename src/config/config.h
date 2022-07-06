#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>

namespace config {

class Config
{
public:
    static Config& instance()
    {
        static Config m_instance;
        return m_instance;
    }

    bool load(const std::string& config_file);

    std::string getLogFile() const {return m_log_file;}
    std::string getPidFile() const {return m_pid_file;}
    std::string getServerIp() const {return m_server_ip;}
    std::string getServerPort() const {return m_server_port;}

    // delete copy and move constructors and assign operators
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(const Config&) = delete;
    Config& operator=(Config&&) = delete;

private:
    Config() {}
    ~Config() {}

private:
    std::string m_config_file;
    std::string m_log_file;
    std::string m_pid_file;
    std::string m_server_ip;
    std::string m_server_port;
};

}

#endif // _CONFIG_H_
