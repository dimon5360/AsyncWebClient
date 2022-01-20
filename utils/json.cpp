/*****************************************************************
 *  @file       json.cpp
 *  @brief      JSON handler class implementation
 *
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @modified   03.09.2021
 *  @version    0.2
 */

 /* local C++ headers */
#include "json.h"

/* external C++ libs headers -------------------------------- */
/* boost C++ lib headers */
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/stream.hpp>

/* spdlog C++ lib */
#include <spdlog/spdlog.h>

/* public method implementation ----------------------------- */

/* TEST JSON data
 * 
 {
    "height" : 320,
    "some" :
    {
        "complex" :
        {
            "path" : "hello"
        }
    },
    "animals" :
    {
        "rabbit" : "white",
        "dog" : "brown",
        "cat" : "grey"
    },
    "fruits" : ["apple", "raspberry", "orange"],
    "matrix" : [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
 }
 * 
 */

#include <fstream>
#include <iostream>

//std::string sTempJson = "{ \"usermail\" : \"test@test.com\", \"username\" : \"test\", \"password\" : \"testPASS2#$\", \"active\" : true }\r\n";

void JsonHandler::PushRequest(std::string&& inReq) const noexcept {
    std::unique_lock lk(mtx_);
    msgQueue.push(std::move(inReq));
}

std::string JsonHandler::PullRequest() const noexcept {
    std::unique_lock lk(mtx_);
    std::string outMsg{ "" };
    if (!msgQueue.empty()) {
        outMsg = msgQueue.front();
        msgQueue.pop();
    }
    return outMsg;
}


void JsonHandler::HandleAuthJson(std::string&& authJson) noexcept {

    namespace pt = boost::property_tree;

    pt::ptree tree = ConstructTree(std::move(authJson));
    auto usermail = ParseTreeParam<std::string>(tree, "usermail");
    //ConsoleLogger::Debug(boost::str(boost::format("%1%") % usermail));
    auto username = ParseTreeParam<std::string>(tree, "username");
    //ConsoleLogger::Debug(boost::str(boost::format("%1%") % username));
    auto password = ParseTreeParam<std::string>(tree, "password");
    //ConsoleLogger::Debug(boost::str(boost::format("%1%") % password));

    // TODO: send to postgres processor
}

void JsonHandler::HandleRequest(std::string&& json, const json_req_t& type) noexcept {

    try {
        switch (type) {
            case json_req_t::authentication_request:
            {
                HandleAuthJson(std::move(json));
                break;
            }
            case json_req_t::user_message:
            {
                HandleAuthJson(std::move(json));
                break;
            }
            default: 
            {
                //throw std::string{boost::str(boost::format("Invalid request type %1%") % type)};
                throw "Invalid request type " + std::to_string(static_cast<double>(type));
            }
        }
    }
    catch (std::exception& ex) {
        //ConsoleLogger::Error(std::string{ ex.what() });
    }
}

boost::property_tree::ptree JsonHandler::ConstructTree(const std::string& jsonString) {
    namespace pt = boost::property_tree;
    pt::ptree ptree;
    boost::iostreams::array_source as(jsonString.data(), jsonString.size());
    boost::iostreams::stream<boost::iostreams::array_source> is(as);
    pt::read_json(is, ptree);
    return ptree;
}

void JsonHandler::PrintTree(boost::property_tree::ptree& tree) {

    /* print separate parameters */
    std::string msg;

    msg = tree.get<std::string>("usermail");
    //ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));
    msg = tree.get<std::string>("username");
    //ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));
    msg = tree.get<std::string>("password");
    //ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));

    /* traversal the tree through 'foreach' */
    for (auto& item : tree) {
        auto key = item.first;
        auto value = item.second.data();
        //ConsoleLogger::Debug(boost::str(boost::format("%1% : %2%") % key % value));
    }
}

template<typename T>
T JsonHandler::ParseJsonParam(const std::string& jsonReq, std::string&& sparam) {
    auto tree = ConstructTree(jsonReq);
    auto param = tree.get<T>(std::move(sparam));
    return param;
}

template<typename T>
T JsonHandler::ParseTreeParam(const boost::property_tree::ptree& jsonTree, std::string&& sparam) {
    auto param = jsonTree.get<T>(std::move(sparam));
    return param;
}


