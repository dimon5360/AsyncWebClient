/*****************************************************************
 *  @file       Logger.h
 *  @brief      Logger class declaration
 *  @author     Kalmykov Dmitry
 *  @date       28.04.2021
 *  @modified   19.08.2021
 *  @version    0.1
 */
#pragma once

#define DEBUG_ENABLE


class ConsoleLogger {

    /* get time code */
    static uint64_t GetCurrTimeMs() noexcept;

public:
        
    static void Info(const std::string &&log) noexcept;
    static void Debug(const std::string &&log) noexcept;
    static void Error(const std::string&& log) noexcept;
};
