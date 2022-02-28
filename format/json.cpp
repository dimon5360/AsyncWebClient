/*****************************************************************
 *  @file       json.cpp
 *  @brief      JSON handler class implementation
 *
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @version    0.2
 */

#include "json.h"
#include "log/logger.h"

#include <fstream>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/date_time.hpp>

#include <spdlog/spdlog.h>

std::string JsonHandler::msg_identificator_token{ "message_identifier" };
std::string JsonHandler::dst_user_msg_token{ "dst_user_id" };
std::string JsonHandler::src_user_msg_token{ "src_user_id" };
std::string JsonHandler::user_msg_token{ "user_message" };
std::string JsonHandler::msg_timestamp_token{ "message_timestamp" };
std::string JsonHandler::msg_hash_token{ "message_hash" };

std::string JsonHandler::users_amount_token{ "users_amount" };
std::string JsonHandler::users_list_token{ "users_list" };

std::string JsonHandler::auth_status_token{ "auth_status" };

/* structure of users list request message
{
    "message_identifier" : users_list_message // details (JsonHandler::json_req_t)
    "src_user_id" : async connection ID
    "message_timestamp" : system datetime
    "message_hash" : sha512 | sha256
}
*/

/* structure of users list response message
{
    "message_identifier" : users_list_message // details (JsonHandler::json_req_t)
    "dst_user_id" : async connection ID
    "users_amount" : 1 ... N, // size of comtainer in UsersPool class (size_t)
    "users_list" : [ "user1_id", "user2_id", ... "userN_id" ],
}
*/

/* structure of message to another user
{
    "message_identifier" : user_message
    "src_user_id" : async connection ID
    "dst_user_id" : async connection ID
    "user_message" : " ... ",
    "message_timestamp" : system datetime
    "message_hash" : sha512 | sha256
}
*/

/* structure of message to users group
{
    "message_identifier" : group_users_message
    "src_user_id" : async connection ID
    "dst_user_id" : [] async connection IDs
    "user_message" : " ... ",
    "message_timestamp" : system datetime
    "message_hash" : sha512 | sha256
}
*/

/* structure of server auth request
{
    "message_identifier" : authentication_message
    "user_message" : " ${login}+${password} ",
    "message_timestamp" : system datetime
    "message_hash" : sha512 | sha256
}
*/

/* structure of server auth response
{
    "message_identifier" : authentication_message
    "dst_user_msg_token" : user ID in server side,
    "auth_status" : "approved" | "denied"
    "message_timestamp" : system datetime
}
*/

#include <openssl/sha.h>
#include <boost/date_time/posix_time/posix_time.hpp>

boost::property_tree::ptree JsonHandler::ConstructTree(JsonHandler::json_req_t jsonMsgType, std::string&& userAuthData) {
    namespace pt = boost::property_tree;
    pt::ptree ptree;

    using namespace boost::posix_time;
    ptree.put(msg_identificator_token, std::to_string(static_cast<int>(jsonMsgType)));
    ptree.put(user_msg_token, userAuthData);
    ptree.put(msg_timestamp_token, to_simple_string(second_clock::local_time()));
    ptree.put(msg_hash_token, ""); // TODO:
    return ptree;
}

boost::property_tree::ptree JsonHandler::ConstructTreeFromJson(const std::string& jsonString) {
    namespace pt = boost::property_tree;
    pt::ptree ptree;
    boost::iostreams::array_source as(jsonString.data(), jsonString.size());
    boost::iostreams::stream<boost::iostreams::array_source> is(as);
    pt::read_json(is, ptree);
    return ptree;
}


std::string JsonHandler::ConvertToString(const boost::property_tree::ptree& jsonTree) const noexcept {

    std::ostringstream oss;
    boost::property_tree::json_parser::write_json(oss, jsonTree);
    return oss.str();
}


