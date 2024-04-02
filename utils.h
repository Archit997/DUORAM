#ifndef UTILS_H
#define UTILS_H

#include<bits/stdc++.h>
using namespace std;

class Matrix; // Assuming the Matrix class is defined in a separate file or later in the source file.
class PRGKey; // Assuming the PRGKey class is defined in a separate file or later in the source file.

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
    std::vector<Matrix*> data;
    
    uint64_t Size();
};

class MsgSlice {
public:
    vector<Msg> data;
    
    uint64_t Size();
};

State MakeState(vector<Matrix*> elems);

CompressedState MakeCompressedState(PRGKey* elem);

Msg MakeMsg(vector<Matrix*> elems);

MsgSlice MakeMsgSlice(vector<Msg> elems);

uint64_t Base_p(uint64_t p, uint64_t m, uint64_t i);

uint64_t Reconstruct_from_base_p(uint64_t p, vector<uint64_t> vals);

uint64_t Compute_num_entries_base_p(uint64_t p, uint64_t log_q);

tuple<uint64_t, uint64_t, uint64_t> Num_DB_entries(uint64_t N, uint64_t row_length, uint64_t p);

double avg(std::vector<double> data);

double stddev(std::vector<double> data);

#endif // UTILS_H
