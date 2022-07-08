#pragma once

#include <memory>
#include <vector>
#include <list>
#include <sstream>

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
    using HandlerType = typename Protocol::HandlerType;
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

    boost::asio::any_io_executor GetExecutor()
    {
        return socket_.get_executor();
    }

    boost::asio::ip::tcp::socket& GetSocket()
    {
        return socket_;
    }

    std::string GetPeerAddress() const
    {
        std::ostringstream ss;
        ss << socket_.remote_endpoint();
        return ss.str();
    }

private:
    void DoRead()
    {
        auto buffer_view = boost::asio::dynamic_buffer(buff_);
        constexpr std::size_t max_buff_size = 8192;

        auto self(this->shared_from_this());
        socket_.async_read_some(buffer_view.prepare(max_buff_size),
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
            {
                LOG_TRACE("%s receive %lu bytes", GetPeerAddress().c_str(), bytes_transferred);
                if (!ec)
                {
                    // process all received request
                    while (buff_.size() > 0)
                    {
                        ParseResultType parse_result = ParseResultType::BAD;
                        std::size_t used_bytes = 0;
                        std::tie(parse_result, used_bytes) =
                            protocol_.Parse(request_, buff_.data(), buff_.size());
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
                            LOG_INFO("receive request %s", request_.to_string().c_str());
                            ResponseType response;
                            handler_.handle(request_, response);
                            Item item;
                            protocol_.Serialize(response, *item.streambuf);
                            item.response = std::move(response);

                            // print serialize result
                            auto buf = item.streambuf->data();
                            const char* ptr = boost::asio::buffer_cast<const char*>(buf);
                            std::string str(ptr, buf.size());
                            LOG_TRACE("serialize to %s", str.c_str());

                            bool write_in_progress = !output_queue_.empty();
                            output_queue_.push_back(std::move(item));
                            if (!write_in_progress)
                            {
                                DoWrite();
                            }
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
                    if (ec == boost::asio::error::eof)
                    {
                        LOG_INFO("peer %s close connection", GetPeerAddress().c_str());
                    }
                    else
                    {
                        LOG_ERROR("read socket error %s", ec.message().c_str());
                    }

                    connection_manager_.Stop(this->shared_from_this());
                }
            });
    }

    void DoWrite()
    {
        auto self(this->shared_from_this());
        const Item& item = output_queue_.front();
        boost::asio::async_write(socket_, *item.streambuf,
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
            {
                if (!ec)
                {
                    LOG_INFO("send response %s", output_queue_.front().response.to_string().c_str());
                    output_queue_.pop_front();
                    if (!output_queue_.empty())
                    {
                        DoWrite();
                    }
                }
                else
                {
                    LOG_ERROR("%s write socket data error %s",
                        GetPeerAddress().c_str(), ec.message().c_str());
                    connection_manager_.Stop(this->shared_from_this());
                }
            });
    }

    struct Item
    {
        ResponseType response;
        std::shared_ptr<boost::asio::streambuf> streambuf = std::make_shared<boost::asio::streambuf>();
    };

    boost::asio::ip::tcp::socket socket_;
    ConnectionManager<Connection>& connection_manager_;
    std::vector<char> buff_; // input data buffer
    Protocol protocol_;
    RequestType request_;
    HandlerType handler_;
    std::list<Item> output_queue_;
};

} // namespace network
