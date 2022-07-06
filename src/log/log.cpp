#include "log.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <array>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>

#include <boost/log/core.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/filesystem.hpp>


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

namespace logger {

static std::string log_file_name_suffix;
static size_t log_file_rotation_size; /**< log file rotation size, unit M */
//g++ -g -DBOOST_LOG_DYN_LINK -lboost_thread -lboost_system -lboost_log -lboost_log_setup -lpthread log.cpp -o log
static const size_t record_queue_limit = 100000;

static boost::shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > g_console_sink;

BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, src::severity_logger_mt)
{
    src::severity_logger_mt<severity_level> lg;
    return lg;
}

// Define the attribute keywords

//we can use attribute by an attribute keyword or name and type
//severity
//expr::attr< severity_level >("Severity")
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)

BOOST_LOG_ATTRIBUTE_KEYWORD(process_id, "ProcessID", attrs::current_process_id::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID", attrs::current_thread_id::value_type)
//BOOST_LOG_ATTRIBUTE_KEYWORD(scope, "Scope", attrs::named_scope::value_type)

// The formatting logic for the severity level
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
    static const char* const str[] =
    {
        "trace",
        "debug",
        "info",
        "warn",
        "error",
        "fatal"
    };
    if (static_cast<std::size_t>(lvl) < (sizeof(str) / sizeof(*str)))
        strm << str[lvl];
    else
        strm << static_cast<int>(lvl);
    return strm;
}

void set_log_level(severity_level level)
{
    logging::core::get()->set_filter(severity >= level);
}

void open_console_log()
{
    //logging::add_console_log(std::clog, keywords::format = "[%TimeStamp%]: %Message%");
    
    // Create a backend and attach a couple of streams to it
    boost::shared_ptr< sinks::text_ostream_backend > backend =
        boost::make_shared< sinks::text_ostream_backend >();

    // add console log
    backend->add_stream(
        boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter()));

    // Enable auto-flushing after each log record written
    backend->auto_flush(true);

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > sink_t;
    boost::shared_ptr< sink_t > sink(new sink_t(backend));
    sink->set_formatter
        (
            expr::stream
            << "["
            << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            << "]"
            << " ["
            << expr::attr< severity_level >("Severity")
            << "]"
            << " [pid:"
            << process_id << " tid:" << thread_id
            << "]: "
            << expr::smessage
            );

    boost::shared_ptr< logging::core > core = logging::core::get();
    core->add_sink(sink);

    g_console_sink = sink;
}

void close_console_log()
{
    logging::core::get()->remove_sink(g_console_sink);
}

static void add_file_log(const std::string& log_file_name)
{
    /*
    logging::add_file_log
        (
            keywords::file_name = "log_%Y%m%d%H%M%S.log",
            keywords::filter = expr::is_in_range(severity, debug, error),
            keywords::rotation_size = 1 * 1024 * 1024 * 1024, //1G
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
            keywords::format = "[%TimeStamp%] [%Severity%]: %Message%"
            );
        */

    boost::shared_ptr< sinks::text_file_backend > backend =
        boost::make_shared< sinks::text_file_backend >(
            keywords::file_name = log_file_name + log_file_name_suffix,
            keywords::rotation_size = log_file_rotation_size * 1024 * 1024,
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0)
            );

    backend->auto_flush(true);
    backend->set_open_mode(std::ios_base::app | std::ios_base::ate);

    // Wrap it into the frontend and register in the core.
    typedef sinks::asynchronous_sink<
                    sinks::text_file_backend,
                    sinks::bounded_fifo_queue<
                            record_queue_limit,
                            sinks::drop_on_overflow>
            > sink_t;

    boost::shared_ptr< sink_t > sink(new sink_t(backend));

    sink->set_formatter
        (
            expr::format("[%1%] [%2%] [pid:%3% tid:%4%]: %5%")
            % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            % severity
            //% expr::format_named_scope("Scopes", keywords::format = "%F:%l")
            % process_id
            % thread_id
            % expr::smessage
            );

    // trace <= level < fatal
    sink->set_filter(expr::is_in_range(severity, trace, fatal));

    boost::shared_ptr< logging::core > core = logging::core::get();
    core->add_sink(sink);
}

/**
    * @brief error log used to monitor, only output fatal log
    */
static void add_error_log(const std::string& log_file_name)
{
    logging::add_file_log
        (
            keywords::file_name = log_file_name + log_file_name_suffix,
            keywords::open_mode = std::ios_base::app | std::ios_base::ate,
            keywords::rotation_size = log_file_rotation_size * 1024 * 1024,
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
            keywords::auto_flush = true,
            keywords::filter = severity == fatal,
            keywords::format = "[%TimeStamp%] [%Severity%] [pid:%ProcessID% tid:%ThreadID%]: %Message%"
            );
}

void init_log(const std::string& log_file_name,
              const severity_level log_level,
              const std::string& file_name_suffix,
              const size_t file_rotation_size)
{
    log_file_name_suffix = file_name_suffix;
    log_file_rotation_size = file_rotation_size;
    logging::add_common_attributes();
    // register operator<< for severity_level
    logging::register_simple_formatter_factory< severity_level, char >("Severity");

    open_console_log();
    add_file_log(log_file_name);
    boost::filesystem::path p(log_file_name);
    std::string error_log_file = (p.parent_path()/="error").string();
    add_error_log(error_log_file);
    set_log_level(log_level);
}

void write_log(severity_level level, const char *file, int line, const char *format, ...)
{
    std::array<char, 4096> message_buffer;
    va_list args;
    va_start (args, format);
    vsnprintf(message_buffer.data(), message_buffer.size(), format, args);
    va_end(args);

    boost::filesystem::path p(file);
    LOGGER(level) << p.filename().c_str() << ":" << line << " " << message_buffer.data();
}

} //namespace logger
