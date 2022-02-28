

#include "DataProcess.h"
#include "../conn/SecureAsyncConnection.h"
#include "../conn/User.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <spdlog/spdlog.h>

std::shared_ptr<DataProcessor> DataProcessor::dp_ = nullptr;

std::shared_ptr<DataProcessor> DataProcessor::CreateProcessor() {
    try {
        if(!dp_) {
            dp_ = std::make_shared<DataProcessor>();
        }
    } catch(std::exception &ex) {
        spdlog::error(boost::str(boost::format("%1% %2") % "Prepate AUTH request error: " % ex.what()));
    }
    return dp_;
}
const std::shared_ptr<DataProcessor>& DataProcessor::GetInstance() {
    return dp_;
}

std::string DataProcessor::PrepareAuthRequest(std::string&& authData) const noexcept {
    std::string auth{};
    try {
        auto tree = jsonHandler->ConstructTree(JsonHandler::json_req_t::authentication_message, std::move(authData));
        auth = {jsonHandler->ConvertToString(tree)};
    } catch(std::exception &ex) {
        spdlog::error(boost::str(boost::format("%1% %2") % "Prepate AUTH request error: " % ex.what()));
    }
    return auth;
}

static std::string authStatucApproved = "approved";
static std::string authStatucDenied = "denied";

bool DataProcessor::ProcessAuthResponse(std::string && sjson) const noexcept {

    boost::property_tree::ptree tree = jsonHandler->ConstructTreeFromJson(sjson);
    bool isUserAuthenticated = false;

    using req_t = JsonHandler::json_req_t;
    std::string msg_identifier = tree.get<std::string>(JsonHandler::msg_identificator_token);

    if(static_cast<req_t>(boost::lexical_cast<int32_t>(msg_identifier)) != req_t::authentication_message) {
        return isUserAuthenticated;
    }

    try {
        std::string authStatus = tree.get<std::string>(JsonHandler::auth_status_token);
        if(!authStatus.compare(authStatucApproved)) {
            spdlog::info("Authentication approved.");
        } else if(!authStatus.compare(authStatucDenied)) {
            spdlog::error("Authentication denied.");
            return isUserAuthenticated;
        } else {
            spdlog::error("Undefined auth status.");
            return isUserAuthenticated;
        }
        isUserAuthenticated = true;
        auto ownId = boost::lexical_cast<SecureTcpConnection::user_id_t>(tree.get<std::string>(JsonHandler::dst_user_msg_token));
        User::GetInstance()->SetUserId(ownId);

    } catch(std::exception &ex) {
        spdlog::error(boost::str(boost::format("%1% %2") % "Parse JSON message error: " % ex.what()));
    }
    return isUserAuthenticated;
}

void DataProcessor::ParseJsonMessage(std::string && message) const noexcept {

    try {
        boost::property_tree::ptree tree = jsonHandler->ConstructTreeFromJson(std::move(message));
        std::string msg_identifier = tree.get<std::string>(JsonHandler::msg_identificator_token);
        using req_t = JsonHandler::json_req_t;

        req_t type = static_cast<req_t>(boost::lexical_cast<int32_t>(msg_identifier));

        switch(type) {
        case req_t::users_list_message: {
            // TODO:
            break;
        }
        case req_t::user_message: {
            // TODO:
            break;
        }
        case req_t::group_users_message: {
            // TODO:
            break;
        }
        default:
            spdlog::error("Undefined message type");
            break;
        }
    } catch(std::exception &ex) {
        spdlog::error(boost::str(boost::format("%1% %2") % "Parse JSON message error: " % ex.what()));
    }
}

DataProcessor::DataProcessor() {
    spdlog::info("Construct DataProcessor class");
    jsonHandler = std::make_unique<JsonHandler>();
}
DataProcessor::~DataProcessor() {
    spdlog::info("Destruct DataProcessor class");
}
