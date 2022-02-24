
#pragma once

#include <string>
#include <memory>

#include "../format/json.h"


class DataProcessor {

    std::unique_ptr<JsonHandler> jsonHandler;

public:

    void ParseJsonMessage(std::string && message);
    std::string PrepareAuthData(std::string&& authData);

    DataProcessor();
};
