#include "Server.h"

#include <signal.h>

#include <thread>

#include "log/log.h"

namespace server {

Server::Server(const std::string& address, const std::string &port)
    : io_context_pool_(std::thread::hardware_concurrency()),
      signal_set_(io_context_pool_.GetIoContext()),
      acceptor_(io_context_pool_.GetIoContext())
{
    RegisterSignalHandler();

    boost::asio::ip::tcp::resolver resolver(io_context_pool_.GetIoContext());
    boost::asio::ip::tcp::endpoint endpoint =
        *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    Accept();
}

void Server::Run()
{
    io_context_pool_.Run();
}

void Server::RegisterSignalHandler()
{
    signal_set_.add(SIGQUIT);
    signal_set_.add(SIGTERM);
    signal_set_.add(SIGINT);
    signal_set_.async_wait(
        [this](boost::system::error_code /*ec*/, int /*signo*/)
        {
            // The server is stopped by cancelling all outstanding asynchronous
            // operations. Once all operations have finished the io_context::run()
            // call will exit.
            acceptor_.close();
            io_context_pool_.Stop();
        }
    );
}

void Server::Accept()
{
    acceptor_.async_accept(io_context_pool_.GetIoContext(),
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
        {
            // Check whether the server was stopped by a signal before this
            // completion handler had a chance to run.
            if (!acceptor_.is_open())
            {
                return;
            }

            if (!ec)
            {
                LOG_INFO("accept connection %d", socket.remote_endpoint().address().to_string().c_str());
                auto connection = std::make_shared<network::Connection<ProtocolType>>(std::move(socket), connection_manager_);
                connection_manager_.Start(connection);
            }
            else
            {
                LOG_ERROR("accept error, %s", ec.message().c_str());
            }

            Accept();
        }
    );
}

}
