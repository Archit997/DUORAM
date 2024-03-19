#include<bits/stdc++.h>
using namespace std;

struct Elem {
    uint64_t val;
};

class Matrix {
public:
    uint64_t Rows;
    uint64_t Cols;
    std::vector<Elem> Data;

    Matrix(uint64_t rows, uint64_t cols) {
        Rows = rows;
        Cols = cols;
        // Initialize Data with appropriate size
        Data.resize(rows * cols);
    }

    Matrix(uint64_t rows, uint64_t cols, std::vector<Elem> data) {
        Rows = rows;
        Cols = cols;
        Data = data;
    }

    uint64_t Size() {
        return Rows * Cols;
    }

    Matrix MatrixZeros(uint64_t rows, uint64_t cols) {
        Matrix out(rows, cols);
        for (auto& elem : out.Data) {
            elem.val = 0;
        }
        return out;
    }

    void Concat(Matrix& b) {
        if (Cols == 0 && Rows == 0) {
            Cols = b.Cols;
            Rows = b.Rows;
            Data = b.Data;
            return;
        }
        if (Cols != b.Cols) {
            cout << Rows << "-by-" << Cols << " vs. " << b.Rows << "-by-" << b.Cols << endl;
            throw runtime_error("Dimension mismatch");
        }
        Rows += b.Rows;
        Data.insert(Data.end(), b.Data.begin(), b.Data.end());
    }
    
    void AppendZeros(uint64_t n) {
        Matrix zeros = MatrixZeros(n, 1);
        Concat(zeros);
    }

    void ReduceMod(uint64_t p) {
        Elem mod = {p};
        for (auto& elem : Data) {
            elem.val = elem.val % mod.val;
        }
    }

    uint64_t Get(uint64_t i, uint64_t j) {
        if (i >= Rows) {
            throw std::runtime_error("Too many rows!");
        }
        if (j >= Cols) {
            throw std::runtime_error("Too many cols!");
        }
        return Data[i * Cols + j].val;
    }

    void Set(uint64_t val, uint64_t i, uint64_t j) {
        if (i >= Rows) {
            throw std::runtime_error("Too many rows!");
        }
        if (j >= Cols) {
            throw std::runtime_error("Too many cols!");
        }
        Data[i * Cols + j].val = val;
    }

    void MatrixAdd(Matrix& b) {
        if ((Cols != b.Cols) || (Rows != b.Rows)) {
            std::cout << Rows << "-by-" << Cols << " vs. " << b.Rows << "-by-" << b.Cols << std::endl;
            throw std::runtime_error("Dimension mismatch");
        }
        for (uint64_t i = 0; i < Cols * Rows; i++) {
            Data[i].val += b.Data[i].val;
        }
    }

    void Add(uint64_t val) {
        Elem v = {val};
        for (auto& elem : Data) {
            elem.val += v.val;
        }
    }

    void AddAt(uint64_t val, uint64_t i, uint64_t j) {
        if ((i >= Rows) || (j >= Cols)) {
            throw std::runtime_error("Out of bounds");
        }
        Set(Get(i, j) + val, i, j);
    }

    void MatrixSub(Matrix& b) {
        if ((Cols != b.Cols) || (Rows != b.Rows)) {
            std::cout << Rows << "-by-" << Cols << " vs. " << b.Rows << "-by-" << b.Cols << std::endl;
            throw std::runtime_error("Dimension mismatch");
        }
        for (uint64_t i = 0; i < Cols * Rows; i++) {
            Data[i].val -= b.Data[i].val;
        }
    }

    static Matrix MatrixMul(Matrix& a, Matrix& b) {
        if (b.Cols == 1) {
            return MatrixMulVec(a, b);
        }
        if (a.Cols != b.Rows) {
            std::cout << a.Rows << "-by-" << a.Cols << " vs. " << b.Rows << "-by-" << b.Cols << std::endl;
            throw std::runtime_error("Dimension mismatch");
        }
        Matrix out(a.Rows, b.Cols);
        for (uint64_t i = 0; i < a.Rows; i++) {
            for (uint64_t j = 0; j < b.Cols; j++) {
                for (uint64_t k = 0; k < a.Cols; k++) {
                    out.Data[i * b.Cols + j].val += a.Data[i * a.Cols + k].val * b.Data[k * b.Cols + j].val;
                }
            }
        }
        return out;
    }

    static Matrix MatrixMulVec(Matrix& a, Matrix& b) {
        if ((a.Cols != b.Rows) && (a.Cols + 1 != b.Rows) && (a.Cols + 2 != b.Rows)) {
            std::cout << a.Rows << "-by-" << a.Cols << " vs. " << b.Rows << "-by-" << b.Cols << std::endl;
            throw std::runtime_error("Dimension mismatch");
        }
        if (b.Cols != 1) {
            throw std::runtime_error("Second argument is not a vector");
        }
        Matrix out(a.Rows, 1);
        for (uint64_t i = 0; i < a.Rows; i++) {
            for (uint64_t j = 0; j < a.Cols; j++) {
                out.Data[i].val += a.Data[i * a.Cols + j].val * b.Data[j].val;
            }
        }
        return out;
    }

    void Transpose() {
        if (Cols == 1) {
            Cols = Rows;
            Rows = 1;
            return;
        }
        if (Rows == 1) {
            Rows = Cols;
            Cols = 1;
            return;
        }
        Matrix out(Cols, Rows);
        for (uint64_t i = 0; i < Rows; i++) {
            for (uint64_t j = 0; j < Cols; j++) {
                out.Data[j * Rows + i].val = Data[i * Cols + j].val;
            }
        }
        Cols = out.Cols;
        Rows = out.Rows;
        Data = out.Data;
    }


    void Print() {
        std::cout << Rows << "-by-" << Cols << " matrix:" << std::endl;
        for (uint64_t i = 0; i < Rows; i++) {
            for (uint64_t j = 0; j < Cols; j++) {
                std::cout << Data[i * Cols + j].val << " ";
            }
            std::cout << std::endl;
        }
    }
};

Matrix MatrixNew(uint64_t rows, uint64_t cols) {
    Matrix out(rows, cols);
    return out;
}

Matrix MatrixRand(uint64_t rows, uint64_t cols, uint64_t logmod, uint64_t mod) {
    Matrix out(rows, cols);
    uint64_t m = mod;
    if (mod == 0) {
        m = pow(2, logmod);
    }
    for (auto& elem : out.Data) {
        elem.val = rand() % m;
    }
    return out;
}

Matrix MatrixGaussian(uint64_t rows, uint64_t cols) {
    Matrix out(rows, cols);
    for (auto& elem : out.Data) {
        elem.val = rand();
    }
    return out;
}

Matrix MatrixMulTransposedPacked(Matrix& a, Matrix& b, uint64_t basis, uint64_t compression) {
    std::cout << a.Rows << "-by-" << a.Cols << " vs. " << b.Cols << "-by-" << b.Rows << std::endl;
    if (compression != 3 && basis != 10) {
        throw std::runtime_error("Must use hard-coded values!");
    }
    Matrix out(a.Rows, b.Rows);
    for (uint64_t j = 0; j < b.Rows; j++) {
        for (uint64_t i = 0; i < a.Cols; i++) {
            uint64_t val = a.Data[i + j * a.Cols].val;
            for (uint64_t f = 0; f < compression; f++) {
                uint64_t new_val = val % mod;
                uint64_t r = (i * compression + f) + a.Cols * compression * (j % concat);
                uint64_t c = j / concat;
                out.Data[r * out.Cols + c / d].val += new_val << (basis * (c % d));
                val /= mod;
            }
        }
    }
    return out;
}

Matrix MatrixMulVecPacked(Matrix& a, Matrix& b, uint64_t basis, uint64_t compression) {
    if (a.Cols * compression != b.Rows) {
        std::cout << a.Rows << "-by-" << a.Cols << " vs. " << b.Rows << "-by-" << b.Cols << std::endl;
        throw std::runtime_error("Dimension mismatch");
    }
    if (b.Cols != 1) {
        throw std::runtime_error("Second argument is not a vector");
    }
    if (compression != 3 && basis != 10) {
        throw std::runtime_error("Must use hard-coded values!");
    }
    Matrix out(a.Rows + 8, 1);
    for (uint64_t i = 0; i < a.Rows; i++) {
        for (uint64_t j = 0; j < a.Cols; j++) {
            uint64_t val = a.Data[i * a.Cols + j].val;
            for (uint64_t f = 0; f < compression; f++) {
                uint64_t new_val = val % mod;
                uint64_t r = (i * compression + f) + a.Cols * compression * (j % concat);
                uint64_t c = j / concat;
                out.Data[r * out.Cols + c / d].val += new_val << (basis * (c % d));
                val /= mod;
            }
        }
    }
    out.DropLastRows(8);
    return out;
}

void transpose(Matrix& out, Matrix& m) {
    for (uint64_t i = 0; i < m.Rows; i++) {
        for (uint64_t j = 0; j < m.Cols; j++) {
            out.Data[j * m.Rows + i].val = m.Data[i * m.Cols + j].val;
        }
    }
}

void matMul(Matrix& out, Matrix& a, Matrix& b) {
    for (uint64_t i = 0; i < a.Rows; i++) {
        for (uint64_t j = 0; j < b.Cols; j++) {
            for (uint64_t k = 0; k < a.Cols; k++) {
                out.Data[i * b.Cols + j].val += a.Data[i * a.Cols + k].val * b.Data[k * b.Cols + j].val;
            }
        }
    }
}

void matMulVec(Matrix& out, Matrix& a, Matrix& b) {
    for (uint64_t i = 0; i < a.Rows; i++) {
        for (uint64_t j = 0; j < a.Cols; j++) {
            out.Data[i].val += a.Data[i * a.Cols + j].val * b.Data[j].val;
        }
    }
}

int main() {
    // Example usage
    Matrix a = MatrixNew(2, 2);
    a.Data[0].val = 1;
    a.Data[1].val = 2;
    a.Data[2].val = 3;
    a.Data[3].val = 4;

    Matrix b = MatrixNew(2, 2);
    b.Data[0].val = 5;
    b.Data[1].val = 6;
    b.Data[2].val = 7;
    b.Data[3].val = 8;

    Matrix c = MatrixMul(a, b);
    c.Print();

    return 0;
}


