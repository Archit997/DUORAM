#include <bits/stdc++.h>
#include <cstdint>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <tuple>

using namespace std;

// Defines the interface for PIR with preprocessing schemes
class PIR {
public:
    virtual string Name() const = 0;

    virtual Params PickParams(uint64_t N, uint64_t d, uint64_t n, uint64_t logq) const = 0;
    virtual Params PickParamsGivenDimensions(uint64_t l, uint64_t m, uint64_t n, uint64_t logq) const = 0;

    virtual void GetBW(DBinfo info, Params p) const = 0;

    virtual State Init(DBinfo info, Params p) const = 0;
    virtual pair<State, CompressedState> InitCompressed(DBinfo info, Params p) const = 0;
    virtual State DecompressState(DBinfo info, Params p, CompressedState comp) const = 0;

    virtual pair<State, Msg> Setup(Database* DB, State shared, Params p) const = 0;
    virtual pair<State, float> FakeSetup(Database* DB, Params p) const = 0;

    virtual pair<State, Msg> Query(uint64_t i, State shared, Params p, DBinfo info) const = 0;

    virtual Msg Answer(Database* DB, MsgSlice query, State server, State shared, Params p) const = 0;

    virtual uint64_t Recover(uint64_t i, uint64_t batch_index, Msg offline, Msg query, Msg answer, State shared, State client,
                             Params p, DBinfo info) const = 0;

    virtual void Reset(Database* DB, Params p) const = 0;
};
// Run PIR's online phase, with a random preprocessing (to skip the offline phase).
// Gives accurate bandwidth and online time measurements.
tuple<double, double, double, double> RunFakePIR(PIR& pi, Database* DB, Params& p, vector<uint64_t>& i,
                                                 ofstream& f, bool profile) {
    cout << "Executing " << pi.Name() << endl;
    // cout << "Memory limit: " << debug.SetMemoryLimit(numeric_limits<size_t>::max()) << endl;
    // Assuming debug.SetMemoryLimit() and math.MaxInt64 are handled separately

    debug.SetGCPercent(-1);

    uint64_t num_queries = i.size();
    if (DB->Data.Rows / num_queries < DB->Info.Ne) {
        throw runtime_error("Too many queries to handle!");
    }
    State shared_state = pi.Init(DB->Info, p);

    cout << "Setup..." << endl;
    auto [server_state, bw] = pi.FakeSetup(DB, p);
    double offline_comm = bw;
    runtime.GC();

    cout << "Building query..." << endl;
    auto start = chrono::steady_clock::now();
    MsgSlice query;
    for (auto index : i) {
        auto [_, q] = pi.Query(index, shared_state, p, DB->Info);
        query.Data.push_back(q);
    }
    cout<<start;
    double online_comm = static_cast<double>(query.Size() * static_cast<uint64_t>(p.Logq) / (8.0 * 1024.0));
    cout << "\t\tOnline upload: " << online_comm << " KB" << endl;
    bw += online_comm;
    runtime.GC();

    cout << "Answering query..." << endl;
    if (profile) {
        pprof.StartCPUProfile(f);
    }
    start = chrono::steady_clock::now();
    Msg answer = pi.Answer(DB, query, server_state, shared_state, p);
    double elapsed = printTime(start);
    if (profile) {
        pprof.StopCPUProfile();
    }
    double rate = printRate(p, elapsed, i.size());
    double online_down = static_cast<double>(answer.Size() * static_cast<uint64_t>(p.Logq) / (8.0 * 1024.0));
    cout << "\t\tOnline download: " << online_down << " KB" << endl;
    bw += online_down;
    online_comm += online_down;

    runtime.GC();
    debug.SetGCPercent(100);
    pi.Reset(DB, p);

    if (offline_comm + online_comm != bw) {
        throw runtime_error("Should not happen!");
    }

    return make_tuple(rate, bw, offline_comm, online_comm);
}

// Run full PIR scheme (offline + online phases).
tuple<double, double> RunPIR(PIR& pi, Database* DB, Params& p, vector<uint64_t>& i) {
    cout << "Executing " << pi.Name() << endl;

    debug.SetGCPercent(-1);

    uint64_t num_queries = i.size();
    if (DB->Data.Rows / num_queries < DB->Info.Ne) {
        throw runtime_error("Too many queries to handle!");
    }
    uint64_t batch_sz = DB->Data.Rows / (DB->Info.Ne * num_queries) * DB->Data.Cols;
    double bw = 0;

    State shared_state = pi.Init(DB->Info, p);

    cout << "Setup..." << endl;
    auto start = chrono::steady_clock::now();
    auto [server_state, offline_download] = pi.Setup(DB, shared_state, p);
    printTime(start);
    double comm = static_cast<double>(offline_download.Size() * static_cast<uint64_t>(p.Logq) / (8.0 * 1024.0));
    cout << "\t\tOffline download: " << comm << " KB" << endl;
    bw += comm;
    runtime.GC();

    cout << "Building query..." << endl;
    start = chrono::steady_clock::now();
    vector<State> client_state;
    MsgSlice query;
    for (auto index : i) {
        uint64_t index_to_query = index + static_cast<uint64_t>(index) * batch_sz;
        auto [cs, q] = pi.Query(index_to_query, shared_state, p, DB->Info);
        client_state.push_back(cs);
        query.Data.push_back(q);
    }
    runtime.GC();
    printTime(start);
    comm = static_cast<double>(query.Size() * static_cast<uint64_t>(p.Logq) / (8.0 * 1024.0));
    cout << "\t\tOnline upload: " << comm << " KB" << endl;
    bw += comm;
    runtime.GC();

    cout << "Answering query..." << endl;
    start = chrono::steady_clock::now();
    Msg answer = pi.Answer(DB, query, server_state, shared_state, p);
    double elapsed = printTime(start);
    double rate = printRate(p, elapsed, i.size());
    comm = static_cast<double>(answer.Size() * static_cast<uint64_t>(p.Logq) / (8.0 * 1024.0));
    cout << "\t\tOnline download: " << comm << " KB" << endl;
    bw += comm;
    runtime.GC();

    pi.Reset(DB, p);
    cout << "Reconstructing..." << endl;
    start = chrono::steady_clock::now();

    for (size_t index = 0; index < i.size(); ++index) {
        uint64_t index_to_query = i[index] + static_cast<uint64_t>(index) * batch_sz;
        uint64_t val = pi.Recover(index_to_query, static_cast<uint64_t>(index), offline_download,
                                   query.Data[index], answer, shared_state,
                                   client_state[index], p, DB->Info);

        if (DB->GetElem(index_to_query) != val) {
            cout << "Batch " << index << " (querying index " << index_to_query << " -- row should be >= " << DB->Data.Rows / 4
                 << "): Got " << val << " instead of " << DB->GetElem(index_to_query) << endl;
            throw runtime_error("Reconstruct failed!");
        }
    }
    cout << "Success!" << endl;
    printTime(start);

    runtime.GC();
    debug.SetGCPercent(100);
    return make_tuple(rate, bw);
}
// Run full PIR scheme (offline + online phases), where the transmission of the A matrix is compressed.
tuple<double, double> RunPIRCompressed(PIR& pi, Database* DB, Params& p, vector<uint64_t>& i) {
    cout << "Executing " << pi.Name() << endl;

    debug.SetGCPercent(-1);

    uint64_t num_queries = i.size();
    if (DB->Data.Rows / num_queries < DB->Info.Ne) {
        throw runtime_error("Too many queries to handle!");
    }
    uint64_t batch_sz = DB->Data.Rows / (DB->Info.Ne * num_queries) * DB->Data.Cols;
    double bw = 0;

    auto [server_shared_state, comp_state] = pi.InitCompressed(DB->Info, p);
    auto client_shared_state = pi.DecompressState(DB->Info, p, comp_state);

    cout << "Setup..." << endl;
    auto start = chrono::steady_clock::now();
    auto [server_state, offline_download] = pi.Setup(DB, server_shared_state, p);
    printTime(start);
    double comm = static_cast<double>(offline_download.Size() * static_cast<uint64_t>(p.Logq) / (8.0 * 1024.0));
    cout << "\t\tOffline download: " << comm << " KB" << endl;
    bw += comm;
    runtime.GC();

    cout << "Building query..." << endl;
    start = chrono::steady_clock::now();
    vector<State> client_state;
    MsgSlice query;
    for (auto index : i) {
        uint64_t index_to_query = index + static_cast<uint64_t>(index) * batch_sz;
        auto [cs, q] = pi.Query(index_to_query, client_shared_state, p, DB->Info);
        client_state.push_back(cs);
        query.Data.push_back(q);
    }
    runtime.GC();
    printTime(start);
    comm = static_cast<double>(query.Size() * static_cast<uint64_t>(p.Logq) / (8.0 * 1024.0));
    cout << "\t\tOnline upload: " << comm << " KB" << endl;
    bw += comm;
    runtime.GC();

    cout << "Answering query..." << endl;
    start = chrono::steady_clock::now();
    Msg answer = pi.Answer(DB, query, server_state, server_shared_state, p);
    double elapsed = printTime(start);
    double rate = printRate(p, elapsed, i.size());
    comm = static_cast<double>(answer.Size() * static_cast<uint64_t>(p.Logq) / (8.0 * 1024.0));
    cout << "\t\tOnline download: " << comm << " KB" << endl;
    bw += comm;
    runtime.GC();

    pi.Reset(DB, p);
    cout << "Reconstructing..." << endl;
    start = chrono::steady_clock::now();

    for (size_t index = 0; index < i.size(); ++index) {
        uint64_t index_to_query = i[index] + static_cast<uint64_t>(index) * batch_sz;
        uint64_t val = pi.Recover(index_to_query, static_cast<uint64_t>(index), offline_download,
                                  query.Data[index], answer, client_shared_state,
                                  client_state[index], p, DB->Info);
        
        if (DB->GetElem(index_to_query) != val) {
            cout << "Batch " << index << " (querying index " << index_to_query << " -- row should be >= " << DB->Data.Rows / 4
                 << "): Got " << val << " instead of " << DB->GetElem(index_to_query) << endl;
            throw runtime_error("Reconstruct failed!");
        }
    }
    cout << "Success!" << endl;
    printTime(start);

    runtime.GC();
    debug.SetGCPercent(100);
    return make_tuple(rate, bw);
}
