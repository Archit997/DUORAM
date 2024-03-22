#ifndef DATABASE_H
#define DATABASE_H

#include <cstdint>
#include <vector>
#include <tuple>
#include <stdexcept>

// Forward declarations
class Matrix; // Assuming the Matrix class is defined in a separate file or later in the source file.
class Params; // Assuming the Params class is defined in a separate file or later in the source file.

class DBinfo {
public:
    // Public member variables
    uint64_t Num, Row_length, Packing, Ne, X, P, Logq, Basis, Squishing, Cols;

    // Constructor declaration
    DBinfo(uint64_t num = 0, uint64_t row_length = 0, uint64_t packing = 0,
           uint64_t ne = 0, uint64_t x = 0, uint64_t p = 0, uint64_t logq = 0,
           uint64_t basis = 0, uint64_t squishing = 0, uint64_t cols = 0);
};

class Database {
public:
    DBinfo Info;
    Matrix* Data;

    // Constructor and Destructor
    Database();
    ~Database();

    // Member functions
    void Squish();
    void Unsquish();
    uint64_t GetElem(uint64_t i);
};

// Function declarations
uint64_t ReconstructElem(const std::vector<uint64_t>& vals, uint64_t index, const DBinfo& info);

std::tuple<uint64_t, uint64_t> ApproxSquareDatabaseDims(uint64_t N, uint64_t row_length, uint64_t p);

std::tuple<uint64_t, uint64_t> ApproxDatabaseDims(uint64_t N, uint64_t row_length, uint64_t p, uint64_t lower_bound_m);

Database* SetupDB(uint64_t Num, uint64_t row_length, const Params* p);

Database* MakeRandomDB(uint64_t Num, uint64_t row_length, const Params* p);

Database* MakeDB(uint64_t Num, uint64_t row_length, const Params* p, const std::vector<uint64_t>& vals);


#endif // PIR_H
