#include<bits/stdc++.h>
using namespace std;
//Include header files for Matrix and PRGKey

class State {
public:
    vector<Matrix*> data;
};

class CompressedState {
public:
    PRGKey* seed;
};

class Msg {
public:
    vector<Matrix*> data;
    
    uint64_t Size() {
        uint64_t sz = 0;
        for (auto d : data) {
            sz += d->Size();
        }
        return sz;
    }
};

class MsgSlice {
public:
    std::vector<Msg> data;
    
    uint64_t Size() {
        uint64_t sz = 0;
        for (auto d : data) {
            sz += d.Size();
        }
        return sz;
    }
};

State MakeState(vector<Matrix*> elems) {
    State st;
    for (auto elem : elems) {
        st.data.push_back(elem);
    }
    return st;
}

CompressedState MakeCompressedState(PRGKey* elem) {
    CompressedState st;
    st.seed = elem;
    return st;
}

Msg MakeMsg(vector<Matrix*> elems) {
    Msg msg;
    for (auto elem : elems) {
        msg.data.push_back(elem);
    }
    return msg;
}

MsgSlice MakeMsgSlice(vector<Msg> elems) {
    MsgSlice slice;
    for (auto elem : elems) {
        slice.data.push_back(elem);
    }
    return slice;
}

uint64_t Base_p(uint64_t p, uint64_t m, uint64_t i) {
    for (uint64_t j = 0; j < i; j++) {
        m = m / p;
    }
    return (m % p);
}

uint64_t Reconstruct_from_base_p(uint64_t p, vector<uint64_t> vals) {
    uint64_t res = 0;
    uint64_t coeff = 1;
    for (auto v : vals) {
        res += coeff * v;
        coeff *= p;
    }
    return res;
}

uint64_t Compute_num_entries_base_p(uint64_t p, uint64_t log_q) {
    double log_p = log2(p);
    return ceil(log_q / log_p);
}

tuple<uint64_t, uint64_t, uint64_t> Num_DB_entries(uint64_t N, uint64_t row_length, uint64_t p) {
    if (row_length <= log2(p)) {
        uint64_t logp = log2(p);
        uint64_t entries_per_elem = logp / row_length;
        uint64_t db_entries = ceil(N / entries_per_elem);
        if (db_entries == 0 || db_entries > N) {
            std::cout << "Num entries is " << db_entries << "; N is " << N << std::endl;
            throw std::runtime_error("Should not happen");
        }
        return std::make_tuple(db_entries, 1, entries_per_elem);
    }
    
    uint64_t ne = Compute_num_entries_base_p(p, row_length);
    return std::make_tuple(N * ne, ne, 0);
}

double avg(vector<double> data) {
    double sum = 0.0;
    double num = 0.0;
    for (auto elem : data) {
        sum += elem;
        num += 1.0;
    }
    return sum / num;
}

double stddev(vector<double> data) {
    double avg_val = avg(data);
    double sum = 0.0;
    double num = 0.0;
    for (auto elem : data) {
        sum += pow(elem - avg_val, 2);
        num += 1.0;
    }
    double variance = sum / num;

    return sqrt(variance);

}
