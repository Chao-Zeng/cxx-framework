#include "log.h"

#include <iostream>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>

#include <boost/log/core.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/support/date_time.hpp>


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

namespace logger {

static const std::string log_file_name_suffix = "_%Y%m%d%H%M%S.log";
static const size_t log_file_rotation_size = 1 * 1024 * 1024 * 1024; /**< log file rotation size 1G */
//g++ -g -DBOOST_LOG_DYN_LINK -lboost_thread -lboost_system -lboost_log -lboost_log_setup -lpthread log.cpp -o log

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
            keywords::rotation_size = log_file_rotation_size,
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0)
            );

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    typedef sinks::synchronous_sink< sinks::text_file_backend > sink_t;
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

    // debug <= level < critical
    sink->set_filter(expr::is_in_range(severity, debug, fatal));

    boost::shared_ptr< logging::core > core = logging::core::get();
    core->add_sink(sink);
}

/**
 * @brief error log used to monitor, only output fatal log
 */
static void add_error_log()
{
    logging::add_file_log
        (
            keywords::file_name = "error_%Y%m%d%H%M%S.log",
            keywords::rotation_size = log_file_rotation_size,
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
            keywords::filter = severity == fatal,
            keywords::format = "[%TimeStamp%] [%Severity%] [pid:%ProcessID% tid:%ThreadID%]: %Message%"
            );
}

void init_log(const std::string& log_file_name)
{
    logging::add_common_attributes();
    // register operator<< for severity_level
    logging::register_simple_formatter_factory< severity_level, char >("Severity");

    open_console_log();
    add_file_log(log_file_name);
    add_error_log();
    set_log_level(info);
}

} //namespace logger 
