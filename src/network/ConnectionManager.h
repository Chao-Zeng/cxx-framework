#pragma once

#include <set>
#include <memory>

namespace network {

template<typename Connection>
class ConnectionManager
{
public:
    ConnectionManager()
    {}
    ~ConnectionManager()
    {
        StopAll();
    }
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    void Start(const std::shared_ptr<Connection> &connection)
    {
        connections_.insert(connection);
        connection->Start();
    }

    void Stop(const std::shared_ptr<Connection> &connection)
    {
        connections_.erase(connection);
        connection->Stop();
    }

    void StopAll()
    {
        for (auto &connection : connections_)
        {
            connection->Stop();
        }

        connections_.clear();
    }

private:
    std::set<std::shared_ptr<Connection>> connections_;
};

} // namespace network
