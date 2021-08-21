/*********************************************
 *
 *
 */
#pragma once

#include <shared_mutex>
#include <queue>

class MessageBroker {

public:
    using message_t = std::pair<const uint64_t, const std::string>;

    /***********************************************************************************
     *  @brief  Push info about new message {msg} for user {connId} to queue
     *  @param  connId  User ID who must receive message
     *  @param  msg Message itself
     *  @return None
     */
    void PushMessage(const uint64_t& connId, const std::string&& msg) {
        std::unique_lock lk(m_);
        msgQueue.emplace(std::make_pair(connId, msg));
        msgNum++;
    }

    /***********************************************************************************
     *  @brief  Check queue is empty
     *  @return Check result, true if queue is empty
     */
    bool IsQueueEmpty() noexcept {
        std::shared_lock lk(m_);
        return msgNum == 0;
    }

protected:

    /***********************************************************************************
     *  @brief  Pull fisrt message from queue
     *  @return Message info
     */
    const message_t PullMessage() {
        std::unique_lock lk(m_);
        message_t msg{ msgQueue.front() };
        msgQueue.pop();
        msgNum--;
        return msg;
    }

private:
    friend class User;
    std::queue<message_t> msgQueue;
    std::shared_mutex m_;
    std::atomic_size_t msgNum;
};