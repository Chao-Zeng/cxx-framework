#pragma once

#include <tuple>

namespace network {
namespace protocol {

template<typename Request, typename Response>
class Protocol {
public:
    using RequestType = Request;
    using ResponseType = Response;

    enum class ParseResult {
        GOOD,
        BAD,
        NEED_MORE
    };

    virtual std::tuple<ParseResult, std::size_t>
    Parse(RequestType &request, const char *data, std::size_t size) = 0;

protected:
    virtual ~Protocol(){}
};

} // namespace protocol
} // namespace network
