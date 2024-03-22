#include "pir.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <utility>

class SimplePIR {
public:
    std::string Name() const {
        return "SimplePIR";
    }

    Params PickParams(uint64_t N, uint64_t d, uint64_t n, uint64_t logq) {
        Params good_p;
        bool found = false;

        // Iteratively refine p and DB dimensions until tight values are found
        for (uint64_t mod_p = 2; ; mod_p++) {
            uint64_t l, m;
            std::tie(l, m) = ApproxSquareDatabaseDims(N, d, mod_p);

            Params p;
            p.N = n;
            p.Logq = logq;
            p.L = l;
            p.M = m;
            p.PickParams(false, m);

            if (p.P < mod_p) {
                if (!found) {
                    throw std::runtime_error("Error; should not happen");
                }
                good_p.PrintParams();
                return good_p;
            }

            good_p = p;
            found = true;
        }

        // Unreachable in theory; included to avoid compiler warnings.
        throw std::runtime_error("Cannot be reached");
    }

    Params PickParamsGivenDimensions(uint64_t l, uint64_t m, uint64_t n, uint64_t logq) {
        Params p;
        p.N = n;
        p.Logq = logq;
        p.L = l;
        p.M = m;
        p.PickParams(false, m);
        return p;
    }

    Database* ConcatDBs(const std::vector<Database*>& DBs, Params* p) {
        if (DBs.empty()) {
            throw std::runtime_error("Should not happen");
        }

        if (DBs[0]->Info.Num != p->L * p->M) {
            throw std::runtime_error("Not yet implemented");
        }

        auto rows = DBs[0]->Data.Rows; // Assuming Data has a Rows member; adjust as needed
        for (size_t j = 1; j < DBs.size(); ++j) {
            if (DBs[j]->Data.Rows != rows) {
                throw std::runtime_error("Bad input");
            }
        }

        Database* D = new Database();
        D->Data = Matrix(0, 0); // Initialize Data to a zero-sized Matrix
        D->Info = DBs[0]->Info;
        D->Info.Num *= DBs.size();
        p->L *= DBs.size();

        for (const auto& db : DBs) {
            D->Data.Concat(db->SelectRows(0, rows));
        }

        return D;
    }

    void GetBW(const DBinfo& info, const Params& p) {
        double offlineDownload = static_cast<double>(p.L * p.N * p.Logq) / (8.0 * 1024.0);
        std::cout << "\t\tOffline download: " << static_cast<uint64_t>(offlineDownload) << " KB\n";

        double onlineUpload = static_cast<double>(p.M * p.Logq) / (8.0 * 1024.0);
        std::cout << "\t\tOnline upload: " << static_cast<uint64_t>(onlineUpload) << " KB\n";

        double onlineDownload = static_cast<double>(p.L * p.Logq) / (8.0 * 1024.0);
        std::cout << "\t\tOnline download: " << static_cast<uint64_t>(onlineDownload) << " KB\n";
    }

    State Init(const DBinfo& info, const Params& p) {
        Matrix A = MatrixRand(p.M, p.N, p.Logq, 0);
        return MakeState(A);
    }

    std::pair<State, CompressedState> InitCompressed(const DBinfo& info, const Params& p) {
        PRGKey* seed = RandomPRGKey();
        return InitCompressedSeeded(info, p, seed);
    }

    std::pair<State, CompressedState> InitCompressedSeeded(const DBinfo& info, const Params& p, PRGKey* seed) {
        BufPRG bufPrgReader = NewBufPRG(NewPRG(seed));
        return {Init(info, p), MakeCompressedState(seed)};
    }

    State DecompressState(const DBinfo& info, const Params& p, const CompressedState& comp) {
        BufPRG bufPrgReader = NewBufPRG(NewPRG(comp.Seed));
        return Init(info, p);
    }

    std::pair<State, Msg> Setup(Database* DB, const State& shared, const Params& p) {
        Matrix A = shared.Data; // Assuming State.Data is a Matrix
        Matrix H = MatrixMul(DB->Data, A);

        DB->Data.Add(p.P / 2);
        DB->Squish();

        return {MakeState(), MakeMsg(H)};
    }

    std::pair<State, double> FakeSetup(Database* DB, const Params& p) {
        double offlineDownload = static_cast<double>(p.L * p.N * p.Logq) / (8.0 * 1024.0);
        std::cout << "\t\tOffline download: " << static_cast<uint64_t>(offlineDownload) << " KB\n";

        DB->Data.Add(p.P / 2);
        DB->Squish();

        return {MakeState(), offlineDownload};
    }

    std::pair<State, Msg> Query(uint64_t i, const State& shared, const Params& p, const DBinfo& info) {
        Matrix A = shared.Data; // Assuming the first matrix in shared data
        Matrix secret = MatrixRand(p.N, 1, p.Logq, 0);
        Matrix err = MatrixGaussian(p.M, 1);
        Matrix query = MatrixMul(A, secret);
        query.MatrixAdd(err);
        // Assuming query.Data is directly accessible and modifiable
        // Adjust based on your actual data structure and operations
        query.Data[i % p.M] += p.Delta(); // Adjust for actual matrix element access

        if (p.M % info.Squishing != 0) {
            query.AppendZeros(info.Squishing - (p.M % info.Squishing));
        }

        return {MakeState(secret), MakeMsg(query)};
    }

    Msg Answer(Database* DB, const std::vector<Msg>& query, const State& server, const State& shared, const Params& p) {
        Matrix ans; // Assume default initialization creates an empty matrix
        uint64_t num_queries = query.size(); 
        uint64_t batch_sz = DB->Data.Rows() / num_queries; 

        uint64_t last = 0;

        for (size_t batch = 0; batch < query.size(); ++batch) {
            if (batch == num_queries - 1) {
                batch_sz = DB->Data.Rows() - last;
            }
            Matrix a = MatrixMulVecPacked(DB->Data.SelectRows(last, batch_sz),
                                          query[batch].Data,
                                          DB->Info.Basis,
                                          DB->Info.Squishing);
            ans.Concat(a);
            last += batch_sz;
        }

        return MakeMsg(ans);
    }

    uint64_t Recover(uint64_t i, uint64_t batch_index, const Msg& offline, const Msg& query, const Msg& answer,
                     const State& shared, const State& client, const Params& p, const DBinfo& info) {
        Matrix secret = client.Data; // Assuming the first matrix in client data
        Matrix H = offline.Data; // Assuming the first matrix in offline data
        Matrix ans = answer.Data; // Assuming the first matrix in answer data

        uint64_t ratio = p.P / 2;
        uint64_t offset = 0;
        for (uint64_t j = 0; j < p.M; ++j) {
            offset += ratio * query.Data.Get(j, 0);
        }
        offset %= static_cast<uint64_t>(std::pow(2, p.Logq));
        offset = static_cast<uint64_t>(std::pow(2, p.Logq)) - offset;

        uint64_t row = i / p.M;
        Matrix interm = MatrixMul(H, secret);
        ans.MatrixSub(interm);

        std::vector<uint64_t> vals;
        for (uint64_t j = row * info.Ne; j < (row + 1) * info.Ne; ++j) {
            uint64_t noised = static_cast<uint64_t>(ans.Get(j, 0)) + offset;
            uint64_t denoised = p.Round(noised);
            vals.push_back(denoised);
            // Optional: Print reconstruction info here
        }
        ans.MatrixAdd(interm);

        return ReconstructElem(vals, i, info);
    }

    void Reset(Database* DB, const Params& p) {
        DB->Unsquish();
        DB->Data.Sub(p.P / 2);
    }
};