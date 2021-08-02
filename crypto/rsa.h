/*********************************************
 *
 *
 */
#pragma once

 /* std C++ lib headers */
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <iomanip>
#include <random>
#include <cmath>

class RsaRandomGen {
private:
    /* random number generator object */
    std::random_device r;
    /* borders of random numbers */
    int32_t MIN = 0, MAX = 8192;

public:

    RsaRandomGen(int32_t min, int32_t max) :
        MIN(min),
        MAX(max)
    {
        std::cout << "Construct new random numbers generator class\n";
    }

    /* destructor */
    ~RsaRandomGen() {
        std::cout << "Destruct random generator class\n";
    }

    /* getter for new random number to send in server */
    int32_t GenRandomNumber() noexcept;
};

class RSA_Crypto {

private:

    std::pair<int32_t, int32_t> private_key, public_key;
    std::unique_ptr<RsaRandomGen> gen_;

    bool IsPrime(const int32_t prime) const;
    int32_t GCD(int32_t e, int32_t t) const;
    int32_t CalcPublicExp(int32_t t) const;
    int32_t CalcPrivateExp(int32_t e, int32_t t) const;

    int32_t EncryptChar(const int32_t i, const int32_t e, const int32_t n) const;
    int32_t DecryptChar(const int32_t i, const int32_t d, const int32_t n) const;
    int32_t GetPrimeNum(const int32_t p) const;

public:

    /* constructor */
    RSA_Crypto();
    /* destructor */
    ~RSA_Crypto();

    void GenKeysPair(int32_t p);

    std::vector<int32_t> Encrypt(const std::string& msg_);
    std::string Decrypt(const std::vector<int32_t>& msg_);
};