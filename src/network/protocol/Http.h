#pragma once

#include <string>
#include <string_view>
#include <sstream>
#include <unordered_map>
#include <optional>

#include <boost/algorithm/string.hpp>

#include "log/log.h"
#include "Protocol.h"
#include "utils/TemplateHelper.h"


namespace network {
namespace protocol {
namespace http {

struct Header
{
    std::string name;
    std::string value;
};

using Headers = std::unordered_map<std::string, std::string>;
static const std::string CR = "\r";
static const std::string LF = "\n";
static const std::string CRLF = "\r\n";

// case insensitive find header
inline std::optional<Header>
ifind_header(const Headers &headers, const std::string &name)
{
    for (const auto &header : headers)
    {
        if (boost::iequals(header.first, name))
        {
            return std::make_optional(Header{header.first, header.second});
        }
    }
    return std::nullopt;
}

class Request
{
public:
    std::string method;
    std::string uri;
    std::string version;
    Headers headers;
    std::string body;
    std::size_t content_length = 0;

    std::string to_string()
    {
        std::stringstream ss;
        ss << method << " " << uri << " ";
        for (const auto &header : headers)
        {
            ss << header.first << ":" << header.second << " ";
        }
        ss << body;
        return ss.str();
    }
};

class Response
{
public:
    enum class StatusCode
    {
        /* 1xx: Informational - Request received, continuing process */
        Continue                        = 100,
        SwitchingProtocols              = 101,
        /* 2xx: Success - The action was successfully received, understood, and accepted */
        OK                              = 200,
        Created                         = 201,
        Accepted                        = 202,
        NonAuthoritativeInformation     = 203,
        NoContent                       = 204,
        ResetContent                    = 205,
        PartialContent                  = 206,
        /* 3xx: Redirection - Further action must be taken in order to complete the request */
        MultipleChoices                 = 300,
        MovedPermanently                = 301,
        Found                           = 302,
        SeeOther                        = 303,
        NotModified                     = 304,
        UseProxy                        = 305,
        TemporaryRedirect               = 307,
        /* 4xx: Client Error - The request contains bad syntax or cannot be fulfilled */
        BadRequest                      = 400,
        Unauthorized                    = 401,
        PaymentRequired                 = 402,
        Forbidden                       = 403,
        NotFound                        = 404,
        MethodNotAllowed                = 405,
        NotAcceptable                   = 406,
        ProxyAuthenticationRequired     = 407,
        RequestTimeout                  = 408,
        Conflict                        = 409,
        Gone                            = 410,
        LengthRequired                  = 411,
        PreconditionFailed              = 412,
        RequestEntityTooLarge           = 413,
        RequestURITooLarge              = 414,
        UnsupportedMediaType            = 415,
        RequestedRangeNotSatisfiable    = 416,
        ExpectationFailed               = 417,
        /* 5xx: Server Error - The server failed to fulfill an apparently valid request */
        InternalServerError             = 500,
        NotImplemented                  = 501,
        BadGateway                      = 502,
        ServiceUnavailable              = 503,
        GatewayTimeout                  = 504,
        HTTPVersionNotSupported         = 505
    };

    static std::string GetReasonPhrase(const StatusCode& status_code)
    {
        static const std::unordered_map<
                        StatusCode, std::string, util::EnumClassHash> status_reason_map = {
            {StatusCode::Continue                     ,"Continue"},
            {StatusCode::SwitchingProtocols           ,"Switching Protocols"},
            {StatusCode::OK                           ,"OK"},
            {StatusCode::Created                      ,"Created"},
            {StatusCode::Accepted                     ,"Accepted"},
            {StatusCode::NonAuthoritativeInformation  ,"Non-Authoritative Information"},
            {StatusCode::NoContent                    ,"No Content"},
            {StatusCode::ResetContent                 ,"Reset Content"},
            {StatusCode::PartialContent               ,"Partial Content"},
            {StatusCode::MultipleChoices              ,"Multiple Choices"},
            {StatusCode::MovedPermanently             ,"Moved Permanently"},
            {StatusCode::Found                        ,"Found"},
            {StatusCode::SeeOther                     ,"See Other"},
            {StatusCode::NotModified                  ,"Not Modified"},
            {StatusCode::UseProxy                     ,"Use Proxy"},
            {StatusCode::TemporaryRedirect            ,"Temporary Redirect"},
            {StatusCode::BadRequest                   ,"Bad Request"},
            {StatusCode::Unauthorized                 ,"Unauthorized"},
            {StatusCode::PaymentRequired              ,"Payment Required"},
            {StatusCode::Forbidden                    ,"Forbidden"},
            {StatusCode::NotFound                     ,"Not Found"},
            {StatusCode::MethodNotAllowed             ,"Method Not Allowed"},
            {StatusCode::NotAcceptable                ,"Not Acceptable"},
            {StatusCode::ProxyAuthenticationRequired  ,"Proxy Authentication Required"},
            {StatusCode::RequestTimeout               ,"Request Time-out"},
            {StatusCode::Conflict                     ,"Conflict"},
            {StatusCode::Gone                         ,"Gone"},
            {StatusCode::LengthRequired               ,"Length Required"},
            {StatusCode::PreconditionFailed           ,"Precondition Failed"},
            {StatusCode::RequestEntityTooLarge        ,"Request Entity Too Large"},
            {StatusCode::RequestURITooLarge           ,"Request-URI Too Large"},
            {StatusCode::UnsupportedMediaType         ,"Unsupported Media Type"},
            {StatusCode::RequestedRangeNotSatisfiable ,"Requested range not satisfiable"},
            {StatusCode::ExpectationFailed            ,"Expectation Failed"},
            {StatusCode::InternalServerError          ,"Internal Server Error"},
            {StatusCode::NotImplemented               ,"Not Implemented"},
            {StatusCode::BadGateway                   ,"Bad Gateway"},
            {StatusCode::ServiceUnavailable           ,"Service Unavailable"},
            {StatusCode::GatewayTimeout               ,"Gateway Time-out"},
            {StatusCode::HTTPVersionNotSupported      ,"HTTP Version not supported"}
        };

        const auto& it = status_reason_map.find(status_code);
        if (it != status_reason_map.end())
        {
            return it->second;
        }

        return "";
    }

    std::string to_string()
    {
        std::stringstream ss;
        ss << "HTTP/1.1" << " " << static_cast<int>(status_code) << " " << GetReasonPhrase(status_code);
        for (const auto &header : headers)
        {
            ss << header.first << ":" << header.second << " ";
        }
        ss << body;
        return ss.str();
    }

    StatusCode status_code;
    Headers headers;
    std::string body;
};

class Handler
{
public:
    virtual void handle(const Request& request, Response& response) noexcept
    {
        response.status_code = Response::StatusCode::OK;
    }
};

class Http : public Protocol<Request, Response, Handler>
{
public:
    Http()
      : parse_status_(ParseStatus::REQUEST_LINE)
    {}

    std::tuple<ParseResult, std::size_t>
    Parse(Request &request, const char *data, std::size_t size) override
    {
        ParseResult result = ParseResult::NEED_MORE;
        std::size_t totoal_used_bytes = 0;
        std::size_t used_bytes = 0;

        switch (parse_status_)
        {
        case ParseStatus::REQUEST_LINE:
            std::tie(result, used_bytes) = ParseRequestLine(request, data, size);
            totoal_used_bytes += used_bytes;
            if (result == ParseResult::GOOD)
            {
                parse_status_ = ParseStatus::HEADERS;
            }
            else
            {
                return std::make_tuple(result, totoal_used_bytes);
            }
        case ParseStatus::HEADERS:
            std::tie(result, used_bytes) = ParseHeaders(request, data + totoal_used_bytes, size - totoal_used_bytes);
            totoal_used_bytes += used_bytes;
            if (result == ParseResult::GOOD)
            {
                parse_status_ = ParseStatus::BODY;
            }
            else
            {
                return std::make_tuple(result, totoal_used_bytes);
            }
        case ParseStatus::BODY:
            std::tie(result, used_bytes) = ParseBody(request, data + totoal_used_bytes, size - totoal_used_bytes);
            totoal_used_bytes += used_bytes;
            if (result == ParseResult::GOOD)
            {
                Reset();
            }
            return std::make_tuple(result, totoal_used_bytes);
        default:
            LOG_ERROR("unknown parse status %d", parse_status_);
            break;
        }

        return std::make_tuple(ParseResult::BAD, 0);
    }

    void Serialize(const Response &response, boost::asio::streambuf &streambuf) override
    {
        std::ostream os(&streambuf);
        // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
        os << "HTTP/1.1" << " " << static_cast<std::underlying_type_t<Response::StatusCode>>(response.status_code)
           << " " << Response::GetReasonPhrase(response.status_code) << CRLF;

        // Headers
        for (const auto& header : response.headers)
        {
            os << header.first << ":" << header.second << CRLF;
        }

        auto content_length_header = ifind_header(response.headers, "Content-Length");
        if (!content_length_header)
        {
            os << "Content-Length" << ":" << std::to_string(response.body.size()) << CRLF;
        }

        os << CRLF;

        // body
        os << response.body;
    }

private:
    std::tuple<ParseResult, std::size_t>
    ParseRequestLine(Request &request, const char *data, std::size_t size)
    {
        //Request-Line = Method SP Request-URI SP HTTP-Version CRLF
        std::string_view req_str(data, size);
        std::string_view::size_type pos = req_str.find(CRLF);
        if (pos == std::string_view::npos)
        {
            return std::make_tuple(ParseResult::NEED_MORE, 0);
        }
        std::size_t used_bytes = pos + CRLF.size();
        std::string_view request_line(data, pos);

        // method
        pos = request_line.find(" ");
        if (pos == std::string_view::npos)
        {
            return std::make_tuple(ParseResult::BAD, 0);
        }

        std::string method(request_line, 0, pos);
        request_line.remove_prefix(pos + 1);
        // TODO: check method

        // uri
        pos = request_line.find(" ");
        if (pos == std::string_view::npos)
        {
            return std::make_tuple(ParseResult::BAD, 0);
        }
        std::string uri(request_line, 0, pos);
        request_line.remove_prefix(pos + 1);

        // version
        pos = request_line.find("HTTP/");
        if (pos == std::string_view::npos)
        {
            return std::make_tuple(ParseResult::BAD, 0);
        }
        std::string version(request_line);

        request.method = method;
        request.uri = uri;
        request.version = version;
        return std::make_tuple(ParseResult::GOOD, used_bytes);
    }

    std::tuple<ParseResult, std::size_t>
    ParseHeaders(Request &request, const char *data, std::size_t size)
    {
        std::string_view req_str(data, size);
        std::size_t used_bytes = 0;

        while (req_str.size() > 0)
        {
            auto pos = req_str.find(CRLF);
            if (pos == std::string_view::npos)
            {
                return std::make_tuple(ParseResult::NEED_MORE, used_bytes);
            }
            std::size_t header_len = pos;
            if (header_len == 0)
            {
                used_bytes += CRLF.size();
                // if get body length failed, return ParseResult::BAD
                request.content_length = GetBodyLength(request);
                return std::make_tuple(ParseResult::GOOD, used_bytes);
            }
            std::string_view header(req_str.substr(0, pos));
            pos = header.find(":");
            if (pos == std::string_view::npos)
            {
                return std::make_tuple(ParseResult::BAD, used_bytes);
            }
            std::string header_name{header.substr(0, pos)};
            std::string header_value{header.substr(pos + 1)};
            boost::trim(header_name);
            boost::trim(header_value);
            request.headers[header_name] = header_value;

            used_bytes += (header_len + CRLF.size());
            req_str.remove_prefix(header_len + CRLF.size());
        }

        return std::make_tuple(ParseResult::NEED_MORE, used_bytes);
    }

    std::tuple<ParseResult, std::size_t>
    ParseBody(Request &request, const char *data, std::size_t size)
    {
        if (request.content_length == 0)
        {
            return std::make_tuple(ParseResult::GOOD, 0);
        }

        std::size_t remain_len = request.content_length - request.body.size();
        if (remain_len <= size)
        {
            request.body.append(data, remain_len);
            return std::make_tuple(ParseResult::GOOD, remain_len);
        }
        else
        {
            request.body.append(data, size);
            return std::make_tuple(ParseResult::NEED_MORE, size);
        }
    }

    std::size_t GetBodyLength(const Request &request)
    {
        auto content_length_header = ifind_header(request.headers, "Content-Length");
        if (!content_length_header)
        {
            return 0;
        }

        int body_len = 0;
        try
        {
            body_len = std::stoi(content_length_header->value);
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("invalid content-length %s, %s",
                content_length_header->value.c_str(), e.what());
            // throw exception
        }
        return body_len;
    }

    void Reset()
    {
        parse_status_ = ParseStatus::REQUEST_LINE;
    }

    enum class ParseStatus
    {
        REQUEST_LINE,
        HEADERS,
        BODY
    } parse_status_;
};

} // namespace http
} // namespace protocol
} // namespace network
