#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <random>
#include <mutex>
#include <cstdint>
#include <algorithm>

constexpr size_t aesBlockSize = 16;
constexpr size_t bufSize = 8192;

using PRGKey = std::array<uint8_t, aesBlockSize>;

class PRGReader {
private:
    PRGKey Key;
    std::mutex mutex;
    std::vector<uint8_t> iv;
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> dist;

public:
    PRGReader(const PRGKey& key) : Key(key), rng(std::random_device{}()), dist(0, UINT64_MAX) {
        iv.fill(0);
    }

    uint64_t RandInt(uint64_t mod) {
        std::lock_guard<std::mutex> lock(mutex);
        while (true) {
            uint64_t rnd = dist(rng);
            if (rnd < (UINT64_MAX - UINT64_MAX % mod)) {
                return rnd % mod;
            }
        }
    }

    std::mt19937_64& GetRNG() {
        return rng;
    }
};

class BufPRGReader {
private:
    PRGReader* prg;
    std::mutex mutex;
    size_t bytesBuffered;
    std::vector<uint8_t> buffer;

public:
    BufPRGReader(PRGReader* prg) : prg(prg), bytesBuffered(0), buffer(bufSize) {}

    uint64_t Uint64() {
        std::lock_guard<std::mutex> lock(mutex);
        if (bytesBuffered < sizeof(uint64_t)) {
            RefillBuffer();
        }
        uint64_t result = *reinterpret_cast<uint64_t*>(buffer.data());
        bytesBuffered -= sizeof(uint64_t);
        std::copy(buffer.begin() + sizeof(uint64_t), buffer.end(), buffer.begin());
        return result;
    }

private:
    void RefillBuffer() {
        std::lock_guard<std::mutex> lock(mutex);
        bytesBuffered = 0;
        uint64_t remaining = bufSize;
        while (remaining > 0) {
            uint64_t rnd = prg->RandInt(UINT64_MAX);
            uint64_t toCopy = std::min(remaining, sizeof(uint64_t));
            std::memcpy(buffer.data() + bytesBuffered, &rnd, toCopy);
            bytesBuffered += toCopy;
            remaining -= toCopy;
        }
    }
};

class MathRand {
private:
    BufPRGReader* prgReader;

public:
    MathRand(BufPRGReader* prgReader) : prgReader(prgReader) {}

    uint64_t Intn(uint64_t n) {
        return prgReader->Uint64() % n;
    }
};

void init();
PRGKey RandomPRGKey();
PRGReader* RandomPRG();
BufPRGReader* NewBufPRG(PRGReader* prg);

PRGReader* prg = nullptr;
BufPRGReader* bufPrgReader = nullptr;

void init() {
    prg = RandomPRG();
    bufPrgReader = NewBufPRG(prg);
}

PRGKey RandomPRGKey() {
    PRGKey key;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0, 255);
    for (auto& byte : key) {
        byte = dis(gen);
    }
    return key;
}

PRGReader* RandomPRG() {
    return new PRGReader(RandomPRGKey());
}

BufPRGReader* NewBufPRG(PRGReader* prg) {
    return new BufPRGReader(prg);
}