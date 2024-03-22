#include <iostream>
#include <vector>
#include <random>
#include <map>

// Define the CDF table
const std::vector<double> cdf_table = {
    // CDF values here...
};

// Gaussian Sample function
int64_t GaussSample() {
    // Gaussian sampling logic here...
}

int main() {
    // Test function
    std::map<int, int> buckets;
    for (int i = 0; i < 1000000; i++) {
        int64_t sample = GaussSample() + 128;
        buckets[sample]++;
    }

    for (auto it = buckets.begin(); it != buckets.end(); ++it) {
        std::cout << "bucket[" << it->first << "] = " << it->second << std::endl;
    }

    return 0;
}