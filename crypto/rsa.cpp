/*********************************************
 *
 *
 */
 /* std C++ lib headers */
#include <iostream>
#include <random>
#include <cmath>
#include <vector>

/* local C++ headers */
#include "rsa.h"

/* getter for new random number to send in server */
int32_t RsaRandomGen::GenRandomNumber() noexcept {
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int32_t> uniform_dist(MIN, MAX);

    int value = uniform_dist(e1);
    return static_cast<int32_t>(value);
}

bool RSA_Crypto::IsPrime(const int32_t prime) const
{
    int32_t i, j;

    j = (int32_t)sqrt((long double)prime);

    for (i = 2; i <= j; i++) {
        if (prime % i == 0) {
            return false;
        }
    }

    return true;
}

int32_t RSA_Crypto::GCD(int32_t e, int32_t t) const
{
    while (e > 0)
    {
        int32_t temp;

        temp = e;
        e = t % e;
        t = temp;
    }

    return t;
}

int32_t RSA_Crypto::CalcPublicExp(int32_t t) const
{
    int64_t e;

    for (e = 2; e < t; e++)
    {
        if (GCD(e, t) == 1)
        {
            return e;
        }
    }

    return -1;
}

int32_t RSA_Crypto::CalcPrivateExp(int32_t e, int32_t t) const
{
    int64_t d = 0;
    int64_t k = 1;

    while (1)
    {
        k = k + t;

        if (k % e == 0)
        {
            d = (k / e);
            return d;
        }
    }
    return d;
}

std::vector<int32_t> RSA_Crypto::Encrypt(const std::string& msg_)
{
    std::vector<int32_t> res(0, msg_.size());
    for (int32_t i = 0; i < msg_.length(); i++) {
        res.push_back(EncryptChar(msg_[i], public_key.second, public_key.first));
    }
    return res;
}

std::string RSA_Crypto::Decrypt(const std::vector<int32_t>& msg_)
{
    std::string str;
    for (int32_t i = 0; i < msg_.size(); i++) {
        str += DecryptChar(msg_[i], private_key.second, private_key.first);
    }
    return str;
}

int32_t RSA_Crypto::EncryptChar(const int32_t i, const int32_t e, const int32_t n) const
{
    int64_t current, result;

    current = i - 97;
    result = 1;

    for (int32_t j = 0; j < e; j++)
    {
        result = result * current;
        result = result % n;
    }

    return result;
}

int32_t RSA_Crypto::DecryptChar(const int32_t i, const int32_t d, const int32_t n) const
{
    int64_t current, result;

    current = i;
    result = 1;

    for (int32_t j = 0; j < d; j++)
    {
        result = result * current;
        result = result % n;
    }

    return result + 97;
}

int32_t RSA_Crypto::GetPrimeNum(const int32_t p) const {

    int32_t q = gen_->GenRandomNumber();
    while (!IsPrime(q) && q != p) {
        q = gen_->GenRandomNumber();
    }
    return q;
}

void RSA_Crypto::GenKeysPair(int32_t p) {

    int32_t q = GetPrimeNum(p);
    int32_t n = p * q;
    int32_t t = (p - 1) * (q - 1);
    int32_t e = CalcPublicExp(t);
    int32_t d = CalcPrivateExp(e, t);
    public_key = std::make_pair(n, e);
    private_key = std::make_pair(n, d);
}

RSA_Crypto::RSA_Crypto() {
    std::cout << "Construct new RSA crypto class\n";

    /* construct random generator */
    gen_ = std::make_unique<RsaRandomGen>(2500, 3700);
    
    // Generate first prime number // TODO: Генерация сильно простых чисел
    int32_t p = gen_->GenRandomNumber();
    while (!IsPrime(p)) {
        p = gen_->GenRandomNumber();
    }
    // + сеансовый ключ

    /* Generate public and private keys pair */
    GenKeysPair(p);
}

RSA_Crypto::~RSA_Crypto() {

    std::cout << "Destruct RSA crypto class\n";
}