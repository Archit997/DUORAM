#include<bits/stdc++.h>
using namespace std;


struct Params {
    int N;
    int L;
    int M;
    int Logq;
    int P;
};

std::chrono::duration<double> printTime(std::chrono::steady_clock::time_point start) {
    auto elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "\tElapsed: " << std::chrono::duration<double>(elapsed).count() << "s\n";
    return elapsed;
}

double printRate(Params p, std::chrono::duration<double> elapsed, int batch_sz) {
    double rate = std::log2(static_cast<double>(p.P)) * static_cast<double>(p.L * p.M) * static_cast<double>(batch_sz) /
        (8 * 1024 * 1024 * elapsed.count());
    std::cout << "\tRate: " << rate << " MB/s\n";
    return rate;
}

void clearFile(std::string filename) {
    std::ofstream file(filename, std::ios::trunc);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    file << "log(n) log(l) log(m) log(q) rate(MB/s) BW(KB)\n";
}

void writeToFile(Params p, double rate, double bw, std::string filename) {
    std::ofstream file(filename, std::ios::app);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    file << static_cast<int>(std::log2(static_cast<double>(p.N))) << ","
         << static_cast<int>(std::log2(static_cast<double>(p.L))) << ","
         << static_cast<int>(std::log2(static_cast<double>(p.M))) << ","
         << p.Logq << ","
         << rate << ","
         << bw << "\n";
}

int main() {
    return 0;
}

