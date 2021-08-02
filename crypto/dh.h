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

class DhRandomGen {
private:
    /* random number generator object */
    std::random_device r;

public:

    DhRandomGen()
    {
        std::cout << "Construct new random numbers generator class\n";
    }

    ~DhRandomGen() {
        std::cout << "Destruct random generator class\n";
    }

    /* getter for new random number to send in server */
    int32_t GenRandomNumber(int32_t min, int32_t max) noexcept;
};

/* Diffie-Hellman algorithm  */
class DH_Crypto {

private:

    int32_t p, g;
    int32_t common_secret_key, private_key, public_key;

    static bool IsPrime(const int32_t prime);
    void GenPublicKey() noexcept;

public:
    static int32_t GetUniquePrimeNum(const int32_t p, const int32_t min, const int32_t max);
    static int32_t GetRandomPrimeNum(const int32_t min, const int32_t max);

    int32_t Calc(const int32_t number, const int32_t degree, const int32_t divider) const;
    int32_t GetPublicKey() const;
    void SetPublicKey(const int32_t ClientPublicKey);

    DH_Crypto(int32_t p_, int32_t g_);
    ~DH_Crypto();
};