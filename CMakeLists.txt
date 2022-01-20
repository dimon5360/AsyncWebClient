cmake_minimum_required(VERSION 3.10)

project(asyncWebClient)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED COMPONENTS chrono filesystem thread system regex)
find_package(spdlog REQUIRED)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

FIND_LIBRARY(CRYPTOPP crypto++ /usr/lib) ## location of libcryptopp.so or libcryptopp.a

find_library(PQXX_LIB pqxx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -lpqxx -lcryptopp")

set(SRC_LIST main.cpp conn/SecureAsyncConnection.cpp conn/User.cpp crypto/dh.cpp crypto/rsa.cpp test/test.cpp utils/json.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} OpenSSL::Crypto OpenSSL::SSL cryptopp)