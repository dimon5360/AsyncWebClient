/*****************************************************************
 *  @file       json.h
 *  @brief      JSON handler class declaration
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @version    0.2
 */
#pragma once

#include <boost/property_tree/ptree.hpp>

#include <queue>
#include <shared_mutex>

class JsonHandler {

public:

    using json_req_t = enum class req_t : uint32_t {
        users_list_message = 1,
        authentication_message,
        user_message,
        group_users_message,
    };
    
    static std::string msg_identificator_token;
    static std::string dst_user_msg_token;
    static std::string src_user_msg_token;
    static std::string user_msg_token;
    static std::string msg_timestamp_token;
    static std::string msg_hash_token;  

    static std::string users_amount_token;
    static std::string users_list_token;

    static std::string auth_status_token;

public:
    boost::property_tree::ptree ConstructTreeFromJson(const std::string& jsonString);
    boost::property_tree::ptree ConstructTree(json_req_t MessageType, std::string&& userAuthData);
    std::string ConvertToString(const boost::property_tree::ptree& jsonTree) const noexcept;

private:
    mutable std::queue<std::string> msgQueue;
    mutable std::shared_mutex mtx_;
};
