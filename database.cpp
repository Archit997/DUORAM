#include <cstdint> // For uint64_t
#include <iostream>
#include <cmath>
#include <tuple>
#include <stdexcept>

// CONVERT MATRIX.GO INTO CPP and CREATE HEADER FILE AND IMPORT INTO THIS FILE
// SIMILARLY DO FOR UTILS.Go


uint64_t Reconstruct_from_base_p(uint64_t p, const std::vector<uint64_t>& vals) {
    uint64_t res = 0;
    uint64_t coeff = 1;
    for (auto v : vals) {
        res += coeff * v;
        coeff *= p;
    }
    return res;
}

uint64_t Base_p(uint64_t p, uint64_t m, uint64_t i) {
    for (uint64_t j = 0; j < i; j++) {
        m /= p;
    }
    return (m % p);
}

uint64_t ReconstructElem(const std::vector<uint64_t>& vals, uint64_t index, const DBinfo& info) {
    uint64_t q = 1ULL << info.Logq; // Use 1ULL for 64-bit unsigned literal

    std::vector<uint64_t> modifiedVals = vals; // Copy vals to modify them

    for (size_t i = 0; i < modifiedVals.size(); ++i) {
        modifiedVals[i] = (modifiedVals[i] + info.P / 2) % q;
        modifiedVals[i] = modifiedVals[i] % info.P;
    }

    uint64_t val = Reconstruct_from_base_p(info.P, modifiedVals);

    if (info.Packing > 0) {
        val = Base_p(1ULL << info.Row_length, val, index % info.Packing);
    }

    return val;
}

class DBinfo {
public:
    uint64_t Num;        // Number of DB entries.
    uint64_t Row_length; // Number of bits per DB entry.

    uint64_t Packing;    // Number of DB entries per Z_p elem, if log(p) > DB entry size.
    uint64_t Ne;         // Number of Z_p elems per DB entry, if DB entry size > log(p).

    uint64_t X;          // Tunable parameter that governs communication,
                         // must be in range [1, Ne] and must be a divisor of Ne;
                         // represents the number of times the scheme is repeated.
    uint64_t P;          // Plaintext modulus.
    uint64_t Logq;       // (Logarithm of) ciphertext modulus.

    // For in-memory DB compression
    uint64_t Basis;
    uint64_t Squishing;
    uint64_t Cols;

    // Constructor (optional)
    DBinfo(uint64_t num = 0, uint64_t row_length = 0, uint64_t packing = 0,
           uint64_t ne = 0, uint64_t x = 0, uint64_t p = 0, uint64_t logq = 0,
           uint64_t basis = 0, uint64_t squishing = 0, uint64_t cols = 0)
        : Num(num), Row_length(row_length), Packing(packing), Ne(ne),
          X(x), P(p), Logq(logq), Basis(basis), Squishing(squishing), Cols(cols) {}
};

class Matrix;

class Database {
public:
    DBinfo Info;
    Matrix* Data; // Pointer to Matrix

    // Constructor
    Database() : Data(nullptr) {} // Initializes Data pointer to nullptr

    // Destructor to handle cleanup
    ~Database() {
        delete Data; // Ensure proper deletion of the Matrix object
        Data = nullptr; // Avoid dangling pointer
    }

    // Copy constructor and copy assignment operator should be
    // defined if deep copy is required. For simplicity, they are
    // not included here. Be cautious of shallow copies.

    // Optionally, more constructors, member functions, or friend functions can be defined here.
    void Squish() {
        // std::cout << "Original DB dims: ";
        // Data->Dim(); // Assuming Dim is a method that prints dimensions

        Info.Basis = 10;
        Info.Squishing = 3;
        Info.Cols = Data->Cols(); // Assuming Cols() method exists in Matrix that returns column count

        Data->Squish(Info.Basis, Info.Squishing);

        // std::cout << "After squishing, with compression factor " << Info.Squishing << ": ";
        // Data->Dim(); // Assuming Dim is again called to print dimensions after squishing

        // Check that params allow for this compression
        if (Info.P > (1ULL << Info.Basis) || Info.Logq < Info.Basis * Info.Squishing) {
            throw std::runtime_error("Bad params");
        }
    }

    void Unsquish() {
        if (Data != nullptr) {
            Data->Unsquish(Info.Basis, Info.Squishing, Info.Cols);
        }
    }

    uint64_t GetElem(uint64_t i) {
        if (i >= Info.Num) {
            throw std::out_of_range("Index out of range");
        }

        uint64_t col = i % Data->Cols(); // Assuming Cols() returns number of columns
        uint64_t row = i / Data->Cols();

        if (Info.Packing > 0) {
            uint64_t new_i = i / Info.Packing;
            col = new_i % Data->Cols();
            row = new_i / Data->Cols();
        }

        std::vector<uint64_t> vals;
        for (uint64_t j = row * Info.Ne; j < (row + 1) * Info.Ne; ++j) {
            vals.push_back(Data->Get(j, col)); // Assuming Get method exists and is public
        }

        return ReconstructElem(vals, i, Info);
    }
};

// Definition for the Matrix class should be provided elsewhere.

// Assuming Num_DB_entries returns a std::tuple<uint64_t, uint64_t, uint64_t>
// The third return value is ignored in this context, similar to the original Go code
std::tuple<uint64_t, uint64_t, uint64_t> Num_DB_entries(uint64_t N, uint64_t row_length, uint64_t p);

std::tuple<uint64_t, uint64_t> ApproxSquareDatabaseDims(uint64_t N, uint64_t row_length, uint64_t p) {
    auto [db_elems, elems_per_entry, _] = Num_DB_entries(N, row_length, p);
    uint64_t l = static_cast<uint64_t>(std::floor(std::sqrt(static_cast<double>(db_elems))));

    uint64_t rem = l % elems_per_entry;
    if (rem != 0) {
        l += elems_per_entry - rem;
    }

    uint64_t m = static_cast<uint64_t>(std::ceil(static_cast<double>(db_elems) / static_cast<double>(l)));

    return std::make_tuple(l, m);
}

std::tuple<uint64_t, uint64_t, uint64_t> Num_DB_entries(uint64_t N, uint64_t row_length, uint64_t p);

std::tuple<uint64_t, uint64_t> ApproxSquareDatabaseDims(uint64_t N, uint64_t row_length, uint64_t p);

std::tuple<uint64_t, uint64_t> ApproxDatabaseDims(uint64_t N, uint64_t row_length, uint64_t p, uint64_t lower_bound_m) {
    auto [l, m] = ApproxSquareDatabaseDims(N, row_length, p);
    if (m >= lower_bound_m) {
        return std::make_tuple(l, m);
    }

    m = lower_bound_m;
    auto [db_elems, elems_per_entry, _] = Num_DB_entries(N, row_length, p);
    l = static_cast<uint64_t>(std::ceil(static_cast<double>(db_elems) / static_cast<double>(m)));

    uint64_t rem = l % elems_per_entry;
    if (rem != 0) {
        l += elems_per_entry - rem;
    }

    return std::make_tuple(l, m);
}


std::tuple<uint64_t, uint64_t, uint64_t> Num_DB_entries(uint64_t Num, uint64_t row_length, uint64_t P);

Database* SetupDB(uint64_t Num, uint64_t row_length, const Params* p) {
    if (Num == 0 || row_length == 0) {
        throw std::runtime_error("Empty database!");
    }

    auto* D = new Database();

    D->Info.Num = Num;
    D->Info.Row_length = row_length;
    D->Info.P = p->P;
    D->Info.Logq = p->Logq;

    auto [db_elems, elems_per_entry, entries_per_elem] = Num_DB_entries(Num, row_length, p->P);
    D->Info.Ne = elems_per_entry;
    D->Info.X = D->Info.Ne;
    D->Info.Packing = entries_per_elem;

    while (D->Info.Ne % D->Info.X != 0) {
        D->Info.X += 1;
    }

    D->Info.Basis = 0;
    D->Info.Squishing = 0;

    double dbSizeMB = (static_cast<double>(p->L) * p->M) * std::log2(static_cast<double>(p->P)) / (1024.0 * 1024.0 * 8.0);
    std::cout << "Total packed DB size is ~" << dbSizeMB << " MB\n";

    if (db_elems > p->L * p->M) {
        delete D; // Clean up before throwing
        throw std::runtime_error("Params and database size don't match");
    }

    if (p->L % D->Info.Ne != 0) {
        delete D; // Clean up before throwing
        throw std::runtime_error("Number of DB elems per entry must divide DB height");
    }

    return D;
}


Database* MakeRandomDB(uint64_t Num, uint64_t row_length, const Params* p) {
    Database* D = SetupDB(Num, row_length, p);
    D->Data = Matrix::Rand(p->L, p->M, 0, p->P); // Generate a random matrix

    // Map DB elems to [-p/2; p/2]
    D->Data->Sub(p->P / 2);

    return D;
}

Database* MakeDB(uint64_t Num, uint64_t row_length, const Params* p, const std::vector<uint64_t>& vals) {
    Database* D = SetupDB(Num, row_length, p);
    D->Data = Matrix::Zeros(p->L, p->M);

    if (vals.size() != Num) {
        delete D; // Cleanup before throwing
        throw std::runtime_error("Bad input DB");
    }

    if (D->Info.Packing > 0) {
        uint64_t at = 0;
        uint64_t cur = 0;
        uint64_t coeff = 1;
        for (size_t i = 0; i < vals.size(); ++i) {
            cur += vals[i] * coeff;
            coeff *= (1ULL << row_length);
            if (((i + 1) % D->Info.Packing == 0) || (i == vals.size() - 1)) {
                D->Data->Set(cur, at / p->M, at % p->M);
                at++;
                cur = 0;
                coeff = 1;
            }
        }
    } else {
        for (size_t i = 0; i < vals.size(); ++i) {
            for (uint64_t j = 0; j < D->Info.Ne; j++) {
                D->Data->Set(Base_p(D->Info.P, vals[i], j), (i / p->M) * D->Info.Ne + j, i % p->M);
            }
        }
    }

    // Map DB elems to [-p/2; p/2]
    D->Data->Sub(p->P / 2);

    return D;
}