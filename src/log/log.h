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
    trace,
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
void init_log(const std::string& log_file_name,
              const std::string& file_name_suffix = "_%Y%m%d.log", /*%Y%m%d%H%M%S*/
              const size_t file_rotation_size = 10*1024 /*unit:M, default 10G*/);

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

/**
 * write log, client should not use this function
 */
void write_log(severity_level level, const char *file, int line, const char *format, ...);

} //namespace logger 

using logger::trace;
using logger::debug;
using logger::info;
using logger::warn;
using logger::error;
using logger::fatal;

/**
 * c++ style log
 */
#define LOGGER(level) BOOST_LOG_SEV(logger::my_logger::get(), level)

/**
 * c style log
 */
#define LOG_TRACE(format, ...) logger::write_log(trace, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) logger::write_log(debug, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) logger::write_log(info, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) logger::write_log(warn, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) logger::write_log(error, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) logger::write_log(fatal, __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif //_LOG_H_
