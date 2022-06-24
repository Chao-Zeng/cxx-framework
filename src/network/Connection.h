#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "log/log.h"
#include "ConnectionManager.h"

namespace network {

template<typename Protocol>
class Connection : public std::enable_shared_from_this<Connection<Protocol>>
{
public:
    using RequestType = typename Protocol::RequestType;
    using ResponseType = typename Protocol::ResponseType;
    //using HandlerType = typename Protocol::HandlerType;
    using ParseResultType = typename Protocol::ParseResult;

    using Ptr = std::shared_ptr<Connection>;

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(boost::asio::ip::tcp::socket socket,
               ConnectionManager<Connection> &connection_manager)
      : socket_(std::move(socket)),
        connection_manager_(connection_manager)
    {}

    void Start()
    {
        DoRead();
    }

    void Stop()
    {
        socket_.close();
    }

private:
    void DoRead()
    {
        auto buffer_view = boost::asio::dynamic_buffer(buff_);
        constexpr std::size_t max_buff_size = 8192;

        auto self = this->shared_from_this();
        socket_.async_read_some(buffer_view.prepare(max_buff_size),
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
            {
                LOG_TRACE("%s receive %lu bytes", socket_.remote_endpoint().address().to_string().c_str(), bytes_transferred);
                if (!ec)
                {
                    // process all received request
                    while (buff_.size() > 0)
                    {
                        ParseResultType parse_result = ParseResultType::BAD;
                        std::size_t used_bytes = 0;
                        std::tie(parse_result, used_bytes) = protocol_.Parse(request_, buff_.data(), buff_.size());
                        auto buffer_view = boost::asio::dynamic_buffer(buff_);
                        buffer_view.consume(used_bytes);
                        
                        if (parse_result == ParseResultType::BAD)
                        {
                            LOG_ERROR("protocol parse error, close connection");
                            connection_manager_.Stop(this->shared_from_this());
                            return;
                        }
                        else if (parse_result == ParseResultType::GOOD)
                        {
                            LOG_DEBUG("receive request %s", request_.to_string().c_str());
                            //handler_.handle(request_);
                        }
                        else if (parse_result == ParseResultType::NEED_MORE)
                        {
                            break;
                        }
                    }

                    DoRead();
                }
                else if (ec != boost::asio::error::operation_aborted)
                {
                    LOG_ERROR("read socket error %s", ec.message().c_str());
                    connection_manager_.Stop(this->shared_from_this());
                }
            });
    }
    void DoWrite();

    boost::asio::ip::tcp::socket socket_;
    ConnectionManager<Connection>& connection_manager_;
    std::vector<char> buff_;
    Protocol protocol_;
    RequestType request_;
    //HandlerType handler_;
};

} // namespace network
