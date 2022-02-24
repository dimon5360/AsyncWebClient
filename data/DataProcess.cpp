

#include "DataProcess.h"

#include <boost/property_tree/json_parser.hpp>

std::string DataProcessor::PrepareAuthData(std::string&& authData) {
    auto tree = jsonHandler->ConstructTree(JsonHandler::json_req_t::authentication_message, std::move(authData));
    std::string auth{jsonHandler->ConvertToString(tree)};
    return auth;
}

void DataProcessor::ParseJsonMessage(std::string && message) {
    auto tree = jsonHandler->ConstructTreeFromJson(std::move(message));
//    jsonHandler->
}

DataProcessor::DataProcessor() {
    jsonHandler = std::make_unique<JsonHandler>();
}
