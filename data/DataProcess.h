
#pragma once

#include <string>
#include <memory>

#include "../format/json.h"

#include <boost/property_tree/json_parser.hpp>

class DataProcessor
{

    std::unique_ptr<JsonHandler> jsonHandler;
    static std::shared_ptr<DataProcessor> dp_;

    // remove copy constructor and copy assignment operator
    DataProcessor(const DataProcessor &copy) = delete;
    DataProcessor &operator=(const DataProcessor &copy) = delete;

public:
    DataProcessor();

    static std::shared_ptr<DataProcessor> CreateProcessor();
    static const std::shared_ptr<DataProcessor> &GetInstance();

    void ParseJsonMessage(std::string &&message) const noexcept;
    std::string PrepareAuthRequest(std::string &&authData) const noexcept;
    std::string PrepareUserMessage(std::string &&userMsg, const uint32_t from,
                                   const uint32_t to) const noexcept;
    bool ProcessAuthResponse(std::string &&sjson) const noexcept;

    ~DataProcessor();
};
