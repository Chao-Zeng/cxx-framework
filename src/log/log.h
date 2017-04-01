#ifndef _LOG_H_
#define _LOG_H_

#include <string>
#include <boost/log/common.hpp>

namespace logger {

/**
 * log severity levels.
 */
enum severity_level
{
    debug,
    info,
    warn,
    error,
    fatal
};

BOOST_LOG_GLOBAL_LOGGER(my_logger, boost::log::sources::severity_logger_mt< severity_level >)

/**
 * init log module
 */
void init_log(const std::string& log_file_name);

/**
 * set log level
 */
void set_log_level(severity_level level);

/**
 * open console level
 */
void open_console_log();

/**
 * close console level
 */
void close_console_log();

} //namespace logger 

using logger::debug;
using logger::info;
using logger::warn;
using logger::error;
using logger::fatal;

#define LOGGER(level) BOOST_LOG_SEV(logger::my_logger::get(), level)

//#define LOG_DEBUG BOOST_LOG_SEV(logger::my_logger::get(), debug) << "[" << __FILE__ << ":" << __LINE__ << "]"

#endif //_LOG_H_