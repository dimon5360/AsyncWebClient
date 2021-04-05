/* std C++ lib headers */
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

/* transform */
#include <algorithm>

/* random */
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>

/* boost C++ lib headers */
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/format.hpp>

/* deployment definitions */
#define CONSOLE_LOGGER      1
#define FILE_LOGGER         1

#if FILE_LOGGER
#include <boost/date_time.hpp>
#include <fstream>
#endif /* FILE_LOGGER */

class file_logger {

private:

#if FILE_LOGGER

    /* file output stream object */
    std::ofstream log_file;
    /* dump file name */
    std::stringstream filename;
    /* mutex object to avoid data race */
    std::mutex _m;

#endif /* FILE_LOGGER */

    /* open log file */
    int open() noexcept {
#if FILE_LOGGER
        log_file.open(filename.str());
        if (!log_file.is_open())
        {
            std::cout << "Log file opening is failed\n\n";
            return 1;
        }
        write(boost::str(boost::format("Log file \'%1%\' is opened.\n") % filename.str()));
#endif /* FILE_LOGGER */
    }

    /* close log file */
    void close() noexcept {
#if FILE_LOGGER
        std::lock_guard<std::mutex> lk(this->_m);
        if (log_file.is_open()) {
            log_file.close();
        }
#endif /* FILE_LOGGER */
    }

    /* get time code */
    uint64_t GetCurrTimeMs() {
        const auto systick_now = std::chrono::system_clock::now();
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
        return nowMs.count();
    }


public:

    /* constructor */
    file_logger() {

#if FILE_LOGGER
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        ptime now = second_clock::local_time();
        filename << "web_client_log " << GetCurrTimeMs() << ".log";
        open();

        write(boost::str(boost::format("Start time \'%1%\' is opened.\n") % to_simple_string(now)));
#endif /* FILE_LOGGER */
    }

    /* destructor */
    ~file_logger() {
        close();
    }

    /* write log string */
    void write(std::string log) noexcept {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        ptime now = second_clock::local_time();
        std::string time = boost::str(boost::format("%1%: ") % to_simple_string(now));
#if CONSOLE_LOGGER
        std::cout << time << log;
#endif /* CONSOLE_LOGGER */
#if FILE_LOGGER
        std::lock_guard<std::mutex> lk(this->_m);
        log_file << time << log;
#endif /* FILE_LOGGER */
    }
};

static file_logger logger;

class RandomGen {
private:
    /* random number generator object */
    std::random_device r;
    /* borders of random numbers */
    const int MIN = 0, MAX = 8192;//1023; // TODO: max value is enough small

public:

    /* constructor */
    RandomGen() {
    }

    /* destructor */
    ~RandomGen() {
        /* ... */
    }

    /* getter for new random number to send in server */
    uint32_t GenRandomNumber() {
        std::default_random_engine e1(r());
        std::uniform_int_distribution<int> uniform_dist(MIN, MAX);

        int value = uniform_dist(e1);
        return static_cast<uint32_t>(value);
    }
};

class tcp_connection
{

private:

    /***********************************************************************************
     *  @brief  Close connection and call the desctructor
     *  @param  error Boost system error object reference
     *  @return None
     */
    void close(const boost::system::error_code& error) {
        shutdown(boost::asio::ip::tcp::socket::shutdown_send, error.value());
        logger.write(boost::str(boost::format("Close connection error: %2% \n") % id_ % error.message()));
        this->~tcp_connection();
    }

    void to_lower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    /***********************************************************************************
     *  @brief  Start process initialization of client
     *  @return None
     */
    void start_init()
    {
        /* hello message to the server */
        std::string msg = "hello server";

        logger.write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

        boost::asio::async_write(socket_, boost::asio::buffer(msg),
            boost::bind(&tcp_connection::handle_init, this,
                boost::asio::placeholders::error));
    }

    /***********************************************************************************
     *  @brief  Callback-handler of async initialization process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void handle_init(const boost::system::error_code& error)
    {
        if (!error) {
            start_auth();
        }
        else {
            close(error);
        }
    }

    /***********************************************************************************
     *  @brief  Start process authentication of client
     *  @return None
     */
    void start_auth()
    {
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&tcp_connection::handle_auth, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    /***********************************************************************************
     *  @brief  Callback-handler of async authentication process
     *  @param  error Boost system error object reference
     *  @param  recvBytes Amount of bytes received from connection
     *  @return None
     */
    void handle_auth(const boost::system::error_code& error,
        std::size_t bytes_transferred)
    {
        if (!error) {

            std::string in_auth_msg{ buf.data(), bytes_transferred };

            logger.write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_auth_msg % bytes_transferred));

            /* support of different register */
            to_lower(in_auth_msg);

            /* check that auth msg corresponds to default value */
            if (in_auth_msg.substr(0, auth_msg.size()).compare(auth_msg) == 0) {
                id_ = in_auth_msg.substr(auth_msg.size());
                start_write();
            }
        }
        else {
            close(error);
        }
    }

    /***********************************************************************************
     *  @brief  Start async reading process from socket
     *  @param  None
     *  @return None
     */
    void start_read()
    {
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&tcp_connection::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    /***********************************************************************************
     *  @brief  Callback-handler of async reading process
     *  @param  error Boost system error object reference
     *  @param  recvBytes Amount of bytes received from connection
     *  @return None
     */
    void handle_read(const boost::system::error_code& error,
        std::size_t bytes_transferred)
    {
        if (!error) {

            std::string in_msg{ buf.data(), bytes_transferred };

            logger.write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_msg % bytes_transferred));

            /* support of different register */
            to_lower(in_msg);

            /* check that auth msg corresponds to default value */
            if (in_msg.substr(0, tech_req_msg.size()).compare(tech_req_msg) == 0) {
                auto number = in_msg.substr(tech_req_msg.size());

                std::stringstream int_conv(number);

                int value;
                int_conv >> value;
                start_write();
            }
        } else {
            close(error);
        }
    }

    /***********************************************************************************
     *  @brief  Start async writing process from socket
     *  @param  None
     *  @return None
     */
    void start_write()
    {
        std::stringstream msg;
        msg << tech_resp_msg << randGen->GenRandomNumber();

        logger.write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg.str() % msg.str().size()));

        boost::asio::async_write(socket_, boost::asio::buffer(msg.str()),
            boost::bind(&tcp_connection::handle_write, this,
                boost::asio::placeholders::error));
    }

    /***********************************************************************************
     *  @brief  Callback-handler of async writing process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void handle_write(const boost::system::error_code& error)
    {
        if (!error) {
            start_read();
        }
        else {
            close(error);
        }
    }

    /* unique pointer to random number generator */
    std::unique_ptr<RandomGen> randGen;
    /* cliet nboost tcp socket object */
    boost::asio::ip::tcp::socket socket_;

    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::resolver::results_type endpoints;

    /* msgs headers to exchange with clients */
    const std::string auth_msg = std::string("hello user id=");
    const std::string tech_msg_header = std::string("user id=");
    const std::string tech_resp_msg = std::string("number=");
    const std::string tech_req_msg = std::string("summ=");

    /* unique id of client */
    std::string id_;

    /* exchange data buffer */
    enum { max_length = 1024 };
    boost::array<char, max_length> buf = { { 0 } };

    /* parameters of host and port to connect */
    const std::string host = "localhost";
    const std::string port = "4059";

public:

    /* constructor, takes boost io_context object reference */
    tcp_connection(boost::asio::io_context& io_context)
        : socket_(io_context),
        resolver_(io_context)
    { 
        try {
            logger.write(boost::str(boost::format("Start connecting to %1%:%2% \n") % host % port));
            endpoints = resolver_.resolve(boost::asio::ip::tcp::v4(), "localhost", "4059");
            boost::asio::ip::tcp::endpoint ep = boost::asio::connect(socket_, endpoints);

            randGen = std::make_unique<RandomGen>();

            start_init();
        }
        catch (std::exception& ex) {
            std::cerr << "Connection failed: " << ex.what() << "\n";
        }
    }

    /* destructor */
    ~tcp_connection() {
        logger.write("Close connection\n");
        socket_.close();
        /* ... */
    }
};

#include <windows.h>

/* separate thread for processing of ESC key press (to close application) */
static void EscapeWait() {
    while (GetAsyncKeyState(VK_ESCAPE) == 0) {
        Sleep(10);
    }
    exit(0);
}

int main() {
    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    logger.write("Press ESC to exit...\n");
    try
    {
        /* separate thread to monitor SPACE key pressing */
        std::thread ext(&EscapeWait);
        /* start tcp client */
        boost::asio::io_context io_context;
        tcp_connection conn(io_context);
        io_context.run();
        ext.join();
    }
    catch (std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
}