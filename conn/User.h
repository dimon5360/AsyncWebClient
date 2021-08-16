
/* local C++ headers */
#include "SecureAsyncConnection.h"

class User {
private:
    std::string username{}, password{};
    bool userValid = false;

    const int MAX_TRY_NUM = 3;

    static std::string SHA256(const std::string& msg);

    static bool CheckValidPassword(const std::string& pass);

    static bool CheckValidUserName(const std::string& username);

    void InputUserName() noexcept;

    void InputPassword() noexcept;

    std::shared_ptr<SecureTcpConnection> conn;
    boost::asio::ssl::context ssl_context;
public:

    bool StartAuthentication();

    void StartInitialization();

    User(boost::asio::io_service&& io_service);
    ~User();

    const std::string GetUserAuthData() const noexcept;

    static void CreateNewUser(boost::asio::io_service& io_service);
};