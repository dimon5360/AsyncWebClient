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
#include "dh.h"

/* getter for new random number to send in server */
int32_t DhRandomGen::GenRandomNumber(int32_t min, int32_t max) noexcept {
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int32_t> uniform_dist(min, max);

    int value = uniform_dist(e1);
    return static_cast<int32_t>(value);
}

bool DH_Crypto::IsPrime(const int32_t prime) {
    int32_t i, j;

    j = (int32_t)sqrt((long double)prime);

    for (i = 2; i <= j; i++) {
        if (prime % i == 0) {
            return false;
        }
    }

    return true;
}

int32_t DH_Crypto::GetRandomPrimeNum(const int32_t min, const int32_t max) {

    static DhRandomGen gen_;
    int32_t v = gen_.GenRandomNumber(min, max);
    while (!IsPrime(v)) {
        v = gen_.GenRandomNumber(min, max);
    }
    return v;
}


int32_t DH_Crypto::GetUniquePrimeNum(const int32_t f, const int32_t min, const int32_t max) {

    static DhRandomGen gen_;
    int32_t v = gen_.GenRandomNumber(min, max);
    while (!IsPrime(v) && v != f) {
        v = gen_.GenRandomNumber(min, max);
    }
    return v;
}

#include <boost/multiprecision/cpp_int.hpp>

int32_t DH_Crypto::Calc(const int32_t number, const int32_t degree, const int32_t divider) const {
    using namespace boost::multiprecision;
    uint1024_t res = number; // TODO: big int
    for (int32_t i = 0; i < degree; i++) {
        res *= number;
    }
    return static_cast<int32_t>(res % divider);
}

void DH_Crypto::SetPublicKey(const int32_t ClientPublicKey) {
    common_secret_key = Calc(ClientPublicKey, private_key, p);
    std::cout << "Server common secret key: " << common_secret_key << std::endl;
}

int32_t DH_Crypto::GetPublicKey() const {
    return public_key;
}

void DH_Crypto::GenPublicKey() noexcept {
    public_key = Calc(g, private_key, p);
}

DH_Crypto::DH_Crypto(int32_t p_, int32_t g_) :
    p(p_),
    g(g_)
{
    std::cout << "Construct new Diffie-Hellman crypto class\n";

    /* construct random generator */
    private_key = GetRandomPrimeNum(100, 150);
    GenPublicKey();
}

DH_Crypto::~DH_Crypto() {

    std::cout << "Destruct Diffie-Hellman crypto class\n";
}