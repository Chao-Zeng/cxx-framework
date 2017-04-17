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
};

}

#endif // _CONFIG_H_
