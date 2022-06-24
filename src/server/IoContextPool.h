#pragma once

#include <cstddef>
#include <vector>
#include <thread>
#include <memory>
#include <stdexcept>
#include <boost/asio.hpp>

namespace server {

class IoContextPool
{
public:
    IoContextPool(const IoContextPool&) = delete;
    IoContextPool& operator=(const IoContextPool&) = delete;

    explicit IoContextPool(std::size_t pool_size)
    {
        if (0 == pool_size)
        {
            throw std::runtime_error("io_context_pool size is 0");
        }
        
        for (std::size_t i = 0; i < pool_size; i++)
        {
            IoContextPtr io_context_ptr = std::make_shared<boost::asio::io_context>();
            io_contexts_.push_back(io_context_ptr);
            work_.push_back(boost::asio::make_work_guard(*io_context_ptr));
        }
    }

    boost::asio::io_context& GetIoContext()
    {
        static std::size_t index = 0;
        if (index >= io_contexts_.size())
        {
            index = 0;
        }

        return *io_contexts_[index++];
    }

    void Run()
    {
        std::vector<std::thread> threads;
        for (std::size_t i = 0; i < io_contexts_.size(); i++)
        {
            threads.emplace_back(std::thread(
                [this, i](){
                    io_contexts_[i]->run();
                }));
        }

        for (auto &thread : threads)
        {
            thread.join();
        }
    }

    void Stop()
    {
        for (auto &io_context : io_contexts_)
        {
            io_context->stop();
        }
    }

private:
    using IoContextPtr = std::shared_ptr<boost::asio::io_context>;
    using IoContextWork =  boost::asio::executor_work_guard<
                                boost::asio::io_context::executor_type>;

    /// The pool of io_contexts.
    std::vector<IoContextPtr> io_contexts_;

    /// The work that keeps the io_contexts running.
    std::vector<IoContextWork> work_;
    
};

}
