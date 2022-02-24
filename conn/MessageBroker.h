/*********************************************
 *
 *
 */
#pragma once

#include <shared_mutex>
#include <queue>
#include <memory>
#include <atomic>

class MessageBroker {

public:

    void PushMessage(std::string&& msg) {
        std::unique_lock lk(m_);
        msgQueue.emplace(std::move(msg));
        msgNum++;
    }

    bool IsQueueEmpty() noexcept {
        std::shared_lock lk(m_);
        return msgNum == 0;
    }

    const std::string PullMessage() {
        std::unique_lock lk(m_);
        std::string msg{ msgQueue.front() };
        msgQueue.pop();
        msgNum--;
        return msg;
    }

private:

    std::queue<std::string> msgQueue;
    std::shared_mutex m_;
    std::atomic_size_t msgNum;
};

extern MessageBroker messageBroker;
