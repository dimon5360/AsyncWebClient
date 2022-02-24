cmake_minimum_required(VERSION 3.5)

project(WebChatClient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)


find_package(Boost REQUIRED COMPONENTS chrono filesystem thread system regex date_time)
find_package(spdlog REQUIRED)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

FIND_LIBRARY(CRYPTOPP crypto++ /usr/lib)
find_library(PQXX_LIB pqxx)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -lpqxx")

set(SRC_LIST conn/SecureAsyncConnection.cpp conn/SecureAsyncConnection.h conn/User.cpp conn/User.h conn/MessageBroker.h
    crypto/dh.cpp crypto/dh.h crypto/rsa.cpp crypto/rsa.h
    format/json.cpp format/json.h
    log/logger.cpp log/logger.h
    data/DataProcess.cpp data/DataProcess.h
    test/test.cpp test/test.h)

add_executable(WebChatClient main.cpp mainwindow.cpp mainwindow.h mainwindow.ui ${SRC_LIST} )
target_link_libraries(WebChatClient PRIVATE Qt5::Widgets ${Boost_LIBRARIES} OpenSSL::Crypto OpenSSL::SSL cryptopp)
