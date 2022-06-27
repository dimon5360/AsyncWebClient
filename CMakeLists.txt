cmake_minimum_required(VERSION 3.10)

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

find_library(CRYPTOPP crypto++ /usr/lib)
find_library(PQXX_LIB pqxx)

find_package(Qt5 COMPONENTS Widgets Core REQUIRED)
if (${Qt5_FOUND})
    message("Found Qt " ${Qt5_VERSION})
else()
    message("Couldn't find Qt")
endif()

# add_subdirectory(forms) @todo: need cmakeslits in subdir
set(SRC_LIST
    core/SecureAsyncConnection.cpp core/SecureAsyncConnection.h 
    core/AppCore.cpp core/AppCore.h data/MessageBroker.h
    crypto/dh.cpp crypto/dh.h crypto/rsa.cpp crypto/rsa.h
    format/json.cpp format/json.h
    log/logger.cpp log/logger.h
    data/DataProcess.cpp data/DataProcess.h
    test/test.cpp test/test.h main.cpp)

set(QT_SRC 
    forms/mainwindow.h
    forms/mainwindow.cpp
    forms/widgetmessages.h
    forms/widgetmessages.cpp
)

set(QT_UI
    forms/mainwindow.ui
    forms/widgetmessages.ui
)
QT5_WRAP_UI( UI_HEADERS ${QT_UI} )

add_executable(WebChatClient ${QT_SRC} ${SRC_LIST} ${UI_HEADERS} )
target_link_libraries(WebChatClient PRIVATE Qt5::Widgets ${Boost_LIBRARIES} OpenSSL::Crypto OpenSSL::SSL cryptopp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -lpqxx")
