#include "config.h"

#include <stdio.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace config {

static const std::string xml_ext = ".xml";
static const std::string json_ext = ".json";
static const std::string ini_ext = ".ini";
static const std::string info_ext = ".info";

bool Config::load(const std::string& config_file)
{
    m_config_file = config_file;

    std::string extension = boost::filesystem::extension(m_config_file);

    boost::property_tree::ptree ptree;

    try
    {
        if (xml_ext == extension)
        {
            boost::property_tree::read_xml(m_config_file, ptree);
        }
        else if (json_ext == extension)
        {
            boost::property_tree::read_json(m_config_file, ptree);
        }
        else if (ini_ext == extension)
        {
            boost::property_tree::read_ini(m_config_file, ptree);
        }
        else if (info_ext == extension)
        {
            boost::property_tree::read_info(m_config_file, ptree);
        }
        else
        {
            return false;
        }
    }
    catch (const boost::property_tree::ptree_error &e)
    {
        printf("read config file error, %s", e.what());
        return false;
    }

    // float v = ptree.get("a.path.to.float.value", -1.f);
    
    m_log_file = ptree.get("log_file", "log/framework.log");

    m_pid_file = ptree.get("pid_file", "framework.pid");

    return true;
}

} // namespace config
