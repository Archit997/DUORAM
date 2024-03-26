// Params.h
#ifndef PARAMS_H
#define PARAMS_H

#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <initializer_list>

extern std::string lwe_params; // Assuming lwe_params is defined elsewhere

class Params {
public:
    uint64_t N;     // LWE secret dimension
    double Sigma;   // LWE error distribution stddev

    uint64_t L;     // DB height
    uint64_t M;     // DB width

    uint64_t Logq;  // (logarithm of) ciphertext modulus
    uint64_t P;     // plaintext modulus

    // Constructor for initializing the Params
    Params();
    Params(uint64_t n, double sigma, uint64_t l, uint64_t m, uint64_t logq, uint64_t p);

    // Methods
    uint64_t Delta() const;
    uint64_t delta() const;
    uint64_t Round(uint64_t x) const;
    void PickParams(bool doublepir, const std::initializer_list<uint64_t>& samples);
    void PrintParams() const;
};

#endif // PARAMS_H
