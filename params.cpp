#include <cmath>    
#include <string> 
#include <sstream>  
#include <iostream>
#include<iostream>

std::string lwe_params;

// Equivalent C++ struct (or class) for the Params struct in Go
class Params {
public:
    uint64_t N;     // LWE secret dimension
    double Sigma;   // LWE error distribution stddev

    uint64_t L;     // DB height
    uint64_t M;     // DB width

    uint64_t Logq;  // (logarithm of) ciphertext modulus
    uint64_t P;     // plaintext modulus

    // Constructor for initializing the Params
    Params(uint64_t n, double sigma, uint64_t l, uint64_t m, uint64_t logq, uint64_t p)
        : N(n), Sigma(sigma), L(l), M(m), Logq(logq), P(p) {}

    // Converted methods from Go
    uint64_t Delta() const {
        return (1ULL << Logq) / P;
    }

    uint64_t delta() const {
        return static_cast<uint64_t>(std::ceil(static_cast<double>(Logq) / std::log2(static_cast<double>(P))));
    }

    uint64_t Round(uint64_t x) const {
        uint64_t DeltaVal = Delta();
        uint64_t v = (x + DeltaVal / 2) / DeltaVal;
        return v % P;
    }

    void PickParams(bool doublepir, const std::initializer_list<uint64_t>& samples) {
        if (N == 0 || Logq == 0) {
            throw std::runtime_error("Need to specify n and q!");
        }

        uint64_t num_samples = 0;
        for (auto ns : samples) {
            if (ns > num_samples) {
                num_samples = ns;
            }
        }

        std::istringstream iss(lwe_params);
        std::string line;
        std::getline(iss, line); // Skip the first line assuming it's a header or similar

        while (std::getline(iss, line)) {
            std::istringstream lineStream(line);
            std::string item;
            std::vector<std::string> lineItems;

            while (std::getline(lineStream, item, ',')) {
                lineItems.push_back(item);
            }

            uint64_t logn = std::stoull(lineItems[0]);
            uint64_t logm = std::stoull(lineItems[1]);
            uint64_t logq = std::stoull(lineItems[2]);

            if ((N == static_cast<uint64_t>(std::pow(2, logn))) &&
                (num_samples <= static_cast<uint64_t>(std::pow(2, logm))) &&
                (Logq == logq)) {
                Sigma = std::stod(lineItems[3]);

                uint64_t mod = std::stoull(lineItems[doublepir ? 6 : 5]);
                P = mod;

                if (Sigma == 0.0 || P == 0) {
                    throw std::runtime_error("Params invalid!");
                }

                return; // Found and set parameters
            }
        }

        std::cerr << "Searched for " << N << ", " << L << "-by-" << M << ", " << Logq << ",\n";
        throw std::runtime_error("No suitable params known!");
    }

    void PrintParams() const {
        int dbSize = static_cast<int>(std::log2(L) + std::log2(M));
        std::cout << "Working with: n=" << N 
                  << "; db size=2^" << dbSize 
                  << " (l=" << L << ", m=" << M << "); logq=" << Logq 
                  << "; p=" << P << "; sigma=" << Sigma << std::endl;
    }
};

