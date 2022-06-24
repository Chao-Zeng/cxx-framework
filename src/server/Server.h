#pragma once

#include <string>

#include <boost/asio.hpp>

#include "IoContextPool.h"
#include "network/ConnectionManager.h"
#include "network/Connection.h"
#include "network/protocol/Http.h"

namespace server {

class Server
{
public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    explicit Server(const std::string &address, const std::string &port);
    void Run();

private:
    void RegisterSignalHandler();
    void Accept();

    IoContextPool io_context_pool_;
    boost::asio::signal_set signal_set_;
    boost::asio::ip::tcp::acceptor acceptor_;
    using ProtocolType = network::protocol::http::Http;
    network::ConnectionManager<network::Connection<ProtocolType>> connection_manager_;
};

} // namespace server
