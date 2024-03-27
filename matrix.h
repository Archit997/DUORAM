#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>

struct Elem {
    uint64_t val;
};

class Matrix {
public:
    uint64_t Rows;
    uint64_t Cols;
    std::vector<Elem> Data;

    Matrix(uint64_t rows, uint64_t cols);
    Matrix(uint64_t rows, uint64_t cols, std::vector<Elem> data);
    uint64_t Size();
    Matrix MatrixZeros(uint64_t rows, uint64_t cols);
    void Concat(Matrix& b);
    void AppendZeros(uint64_t n);
    void ReduceMod(uint64_t p);
    uint64_t Get(uint64_t i, uint64_t j);
    void Set(uint64_t val, uint64_t i, uint64_t j);
    void MatrixAdd(Matrix& b);
    void Add(uint64_t val);
    void AddAt(uint64_t val, uint64_t i, uint64_t j);
    void MatrixSub(Matrix& b);
    static Matrix MatrixMul(Matrix& a, Matrix& b);
    static Matrix MatrixMulVec(Matrix& a, Matrix& b);
    void Transpose();
    void Print();
};

Matrix MatrixNew(uint64_t rows, uint64_t cols);
Matrix MatrixRand(uint64_t rows, uint64_t cols, uint64_t logmod, uint64_t mod);
Matrix MatrixGaussian(uint64_t rows, uint64_t cols);
Matrix MatrixMulTransposedPacked(Matrix& a, Matrix& b, uint64_t basis, uint64_t compression);
Matrix MatrixMulVecPacked(Matrix& a, Matrix& b, uint64_t basis, uint64_t compression);
void transpose(Matrix& out, Matrix& m);
void matMul(Matrix& out, Matrix& a, Matrix& b);
void matMulVec(Matrix& out, Matrix& a, Matrix& b);

#endif // MATRIX_OPERATIONS_H
