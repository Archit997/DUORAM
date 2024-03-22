#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>

constexpr uint64_t LOGQ = 32;
constexpr uint64_t SEC_PARAM = 1 << 10;

struct Params {
    // Parameters struct definition
};

struct Database {
    // Database struct definition
};

class SimplePIR {
public:
    Params PickParams(uint64_t N, uint64_t d, uint64_t secParam, uint64_t logQ) {
        // Parameter picking logic here...
    }

    Database MakeDB(uint64_t N, uint64_t d, Params* p, std::vector<uint64_t>& vals) {
        // Database creation logic here...
    }
    
    Database ConcatDBs(std::vector<Database*>& DBs, Params* p) {
        // Database concatenation logic here...
    }
    
    void GetBW(DatabaseInfo info, Params p) {
        // BW calculation logic here...
    }
};

void TestDBMediumEntries() {
    uint64_t N = 4;
    uint64_t d = 9;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    std::vector<uint64_t> vals = {1, 2, 3, 4};
    Database DB = pir.MakeDB(N, d, &p, vals);

    if (DB.Info.Packing != 1 || DB.Info.Ne != 1) {
        throw std::runtime_error("Should not happen.");
    }

    for (uint64_t i = 0; i < N; i++) {
        if (DB.GetElem(i) != (i + 1)) {
            throw std::runtime_error("Failure");
        }
    }
}

void TestDBSmallEntries() {
    uint64_t N = 4;
    uint64_t d = 3;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    std::vector<uint64_t> vals = {1, 2, 3, 4};
    Database DB = pir.MakeDB(N, d, &p, vals);

    if (DB.Info.Packing <= 1 || DB.Info.Ne != 1) {
        throw std::runtime_error("Should not happen.");
    }

    for (uint64_t i = 0; i < N; i++) {
        if (DB.GetElem(i) != (i + 1)) {
            throw std::runtime_error("Failure");
        }
    }
}

void TestDBLargeEntries() {
    uint64_t N = 4;
    uint64_t d = 12;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    std::vector<uint64_t> vals = {1, 2, 3, 4};
    Database DB = pir.MakeDB(N, d, &p, vals);

    if (DB.Info.Packing != 0 || DB.Info.Ne <= 1) {
        throw std::runtime_error("Should not happen.");
    }

    for (uint64_t i = 0; i < N; i++) {
        if (DB.GetElem(i) != (i + 1)) {
            throw std::runtime_error("Failure");
        }
    }
}

void TestDBInterleaving() {
    uint64_t N = 16;
    uint64_t d = 8;
    uint64_t numBytes = std::strlen("string 16");

    std::vector<Database*> DBs(numBytes);
    SimplePIR pir;
    Params p = pir.PickParams(N, d, 1 << 10, 32);

    for (uint64_t n = 0; n < numBytes; ++n) {
        std::vector<uint64_t> val(N);
        for (uint64_t i = 0; i < N; ++i) {
            const char* str = "string " + std::to_string(i);
            if (std::strlen(str) > n) {
                val[i] = static_cast<uint64_t>(str[n]);
            } else {
                val[i] = 0;
            }
        }
        DBs[n] = new Database(pir.MakeDB(N, d, &p, val));
    }

    Database D = pir.ConcatDBs(DBs, &p);

    for (uint64_t i = 0; i < N; ++i) {
        std::string val;
        for (uint64_t n = 0; n < numBytes; ++n) {
            val += static_cast<char>(D.GetElem(i + N * n));
        }
        std::cout << "Got '" << val << "' instead of 'string " << i << "'" << std::endl;
        if (val != "string " + std::to_string(i)) {
            throw std::runtime_error("Failure");
        }
    }

    // Clean up allocated Database objects
    for (auto db : DBs) {
        delete db;
    }
}

void TestSimplePirBW() {
    uint64_t N = 1 << 20;
    uint64_t d = 2048;

    char* log_N_env = std::getenv("LOG_N");
    char* D_env = std::getenv("D");
    if (log_N_env != nullptr) {
        int log_N = std::atoi(log_N_env);
        if (log_N != 0) {
            N = 1 << log_N;
        }
    }
    if (D_env != nullptr) {
        int D = std::atoi(D_env);
        if (D != 0) {
            d = D;
        }
    }

    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);
    Database DB = pir.SetupDB(N, d, &p);

    std::cout << "Executing with entries consisting of " << d << " (>= 1) bits; p is " << p.P
              << "; packing factor is " << DB.Info.Packing << "; number of DB elems per entry is "
              << DB.Info.Ne << "." << std::endl;

    pir.GetBW(DB.Info, p);
}

void TestDoublePirBW() {
    uint64_t N = 1 << 20;
    uint64_t d = 2048;

    char* log_N_env = std::getenv("LOG_N");
    char* D_env = std::getenv("D");
    if (log_N_env != nullptr) {
        int log_N = std::atoi(log_N_env);
        if (log_N != 0) {
            N = 1 << log_N;
        }
    }
    if (D_env != nullptr) {
        int D = std::atoi(D_env);
        if (D != 0) {
            d = D;
        }
    }

    DoublePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);
    Database DB = pir.SetupDB(N, d, &p);

    std::cout << "Executing with entries consisting of " << d << " (>= 1) bits; p is " << p.P
              << "; packing factor is " << DB.Info.Packing << "; number of DB elems per entry is "
              << DB.Info.Ne << "." << std::endl;

    pir.GetBW(DB.Info, p);
}

void TestSimplePir() {
    uint64_t N = 1 << 20;
    uint64_t d = 8;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {262144});
}

void TestSimplePirCompressed() {
    uint64_t N = 1 << 20;
    uint64_t d = 8;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {262144});
}

void TestSimplePirLongRow() {
    uint64_t N = 1 << 20;
    uint64_t d = 32;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {1});
}

void TestSimplePirLongRowCompressed() {
    uint64_t N = 1 << 20;
    uint64_t d = 32;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {1});
}

void TestSimplePirBigDB() {
    uint64_t N = 1 << 25;
    uint64_t d = 7;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {0});
}

void TestSimplePirBigDBCompressed() {
    uint64_t N = 1 << 25;
    uint64_t d = 7;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {0});
}

void TestSimplePirBatch() {
    uint64_t N = 1 << 20;
    uint64_t d = 8;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {0, 0, 0, 0});
}

void TestSimplePirBatchCompressed() {
    uint64_t N = 1 << 20;
    uint64_t d = 8;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {0, 0, 0, 0});
}

void TestSimplePirLongRowBatch() {
    uint64_t N = 1 << 20;
    uint64_t d = 32;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {0, 0, 0, 0});
}

void TestSimplePirLongRowBatch() {
    uint64_t N = 1 << 20;
    uint64_t d = 32;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIR(&DB, p, {0, 0, 0, 0});
}

void TestSimplePirLongRowBatchCompressed() {
    uint64_t N = 1 << 20;
    uint64_t d = 32;
    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    Database DB = pir.MakeRandomDB(N, d, &p);
    pir.RunPIRCompressed(&DB, p, {0, 0, 0, 0});
}

void BenchmarkSimplePirSingle() {
    uint64_t N = 1 << 20;
    uint64_t d = 2048;

    char* log_N_env = std::getenv("LOG_N");
    char* D_env = std::getenv("D");
    if (log_N_env != nullptr) {
        int log_N = std::atoi(log_N_env);
        if (log_N != 0) {
            N = 1 << log_N;
        }
    }
    if (D_env != nullptr) {
        int D = std::atoi(D_env);
        if (D != 0) {
            d = D;
        }
    }

    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    uint64_t i = 0; // index to query
    if (i >= p.L * p.M) {
        throw std::runtime_error("Index out of dimensions");
    }

    Database DB = pir.MakeRandomDB(N, d, &p);
    std::vector<float64_t> tputs;
    for (int j = 0; j < 5; j++) {
        auto [tput, _, _, _] = pir.RunFakePIR(&DB, p, {i});
        tputs.push_back(tput);
    }

    // Calculate average throughput
    float64_t avg_tput = 0.0;
    for (auto tput : tputs) {
        avg_tput += tput;
    }
    avg_tput /= tputs.size();

    // Print results
    std::cout << "Avg SimplePIR throughput, except for first run: " << avg_tput << " MB/s" << std::endl;
}

void BenchmarkSimplePirVaryingDB() {
    std::ofstream flog("simple-comm.log", std::ios::app);
    if (!flog.is_open()) {
        throw std::runtime_error("Error creating log file");
    }

    flog << "N,d,tput,tput_stddev,offline_comm,online_comm" << std::endl;

    SimplePIR pir;
    int total_sz = 33;

    for (uint64_t d = 1; d <= 32768; d *= 2) {
        uint64_t N = (1 << total_sz) / d;
        Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

        uint64_t i = 0; // index to query
        if (i >= p.L * p.M) {
            throw std::runtime_error("Index out of dimensions");
        }

        Database DB = pir.MakeRandomDB(N, d, &p);
        std::vector<float64_t> tputs;
        std::vector<float64_t> offline_cs;
        std::vector<float64_t> online_cs;

        for (int j = 0; j < 5; j++) {
            auto [tput, _, offline_c, online_c] = pir.RunFakePIR(&DB, p, {i});
            tputs.push_back(tput);
            offline_cs.push_back(offline_c);
            online_cs.push_back(online_c);
        }

        // Calculate average throughput
        float64_t avg_tput = 0.0;
        for (auto tput : tputs) {
            avg_tput += tput;
        }
        avg_tput /= tputs.size();

        // Calculate standard deviation of throughput
        float64_t tput_stddev = 0.0;
        for (auto tput : tputs) {
            tput_stddev += (tput - avg_tput) * (tput - avg_tput);
        }
        tput_stddev = std::sqrt(tput_stddev / tputs.size());

        // Calculate average offline and online communication
        float64_t avg_offline_c = std::accumulate(offline_cs.begin(), offline_cs.end(), 0.0) / offline_cs.size();
        float64_t avg_online_c = std::accumulate(online_cs.begin(), online_cs.end(), 0.0) / online_cs.size();

        // Write results to log file
        flog << N << "," << d << "," << avg_tput << "," << tput_stddev << "," << avg_offline_c << "," << avg_online_c << std::endl;
    }
}

void BenchmarkSimplePirBatchLarge() {
    std::ofstream f("simple-cpu-batch.out");
    if (!f.is_open()) {
        throw std::runtime_error("Error creating file");
    }

    std::ofstream flog("simple-batch.log");
    if (!flog.is_open()) {
        throw std::runtime_error("Error creating log file");
    }

    uint64_t N = 1 << 33;
    uint64_t d = 1;

    SimplePIR pir;
    Params p = pir.PickParams(N, d, SEC_PARAM, LOGQ);

    uint64_t i = 0; // index to query
    if (i >= p.L * p.M) {
        throw std::runtime_error("Index out of dimensions");
    }

    Database DB = pir.MakeRandomDB(N, d, &p);

    flog << "Batch_sz,Good_tput,Good_std_dev,Num_successful_queries,Tput" << std::endl;

    for (int trial = 0; trial <= 10; trial += 1) {
        int batch_sz = (1 << trial);
        std::vector<uint64_t> query(batch_sz, i);
        std::vector<float64_t> tputs;

        for (int iter = 0; iter < 5; iter++) {
            auto [tput, _, _, _] = pir.RunFakePIR(&DB, p, query);
            tputs.push_back(tput);
        }

        double expected_num_empty_buckets = pow(double(batch_sz - 1) / double(batch_sz), double(batch_sz)) * double(batch_sz);
        double expected_num_successful_queries = double(batch_sz) - expected_num_empty_buckets;
        double good_tput = (std::accumulate(tputs.begin(), tputs.end(), 0.0) / tputs.size()) / double(batch_sz) * expected_num_successful_queries;
        double dev = (std::sqrt(std::accumulate(tputs.begin(), tputs.end(), 0.0, 
                            [good_tput](double acc, double val) { return acc + (val - good_tput) * (val - good_tput); }) / tputs.size())) / double(batch_sz) * expected_num_successful_queries;

        flog << batch_sz << ","
            << good_tput << ","
            << dev << ","
            << expected_num_successful_queries << ","
            << (std::accumulate(tputs.begin(), tputs.end(), 0.0) / tputs.size()) << std::endl;
    }
}

