#pragma once

#include <tuple>
#include <boost/asio.hpp>

namespace network {
namespace protocol {

template<typename Request, typename Response, typename Handler>
class Protocol {
public:
    using RequestType = Request;
    using ResponseType = Response;
    using HandlerType = Handler;

    enum class ParseResult {
        GOOD,
        BAD,
        NEED_MORE
    };

    virtual std::tuple<ParseResult, std::size_t>
    Parse(RequestType &request, const char *data, std::size_t size) = 0;

    virtual bool Serialize(const Response &response, boost::asio::streambuf &streambuf) = 0;

protected:
    virtual ~Protocol(){}
};

} // namespace protocol
} // namespace network
