#ifndef _BLOCKING_QUEUE_H_
#define _BLOCKING_QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

namespace util{

class interrupt_error : public std::logic_error
{
public:
    explicit interrupt_error(const std::string &message)
        : std::logic_error(message)
    {
    }

    explicit interrupt_error(const char *message)
        : std::logic_error(message)
    {
    }
};

template<typename T>
class BlockingQueue final
{
public:
    BlockingQueue()
        : mInterrupt(false)
    {
    }
    ~BlockingQueue() = default;
    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;
    BlockingQueue(BlockingQueue&&) = delete;
    BlockingQueue& operator=(BlockingQueue&&) = delete;

    void push(const T &val)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mQueue.emplace(val);
        }

        mCondition.notify_one();
    }

    void push(T &&val)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mQueue.emplace(std::move(val));
        }

        mCondition.notify_one();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mCondition.wait(lock, [this](){
            return (mInterrupt || !mQueue.empty());
        });

        if (mInterrupt)
        {
            throw interrupt_error("interrupted");
        }

        T val(std::move(mQueue.front()));
        mQueue.pop();
        return val;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.size();
    }

    void interrupt()
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mInterrupt = true;
        }
        mCondition.notify_all();
    }

private:
    std::queue<T> mQueue;
    mutable std::mutex mMutex;
    std::condition_variable mCondition;
    bool mInterrupt;
};

} // namespace util

#endif // _BLOCKING_QUEUE_H_
