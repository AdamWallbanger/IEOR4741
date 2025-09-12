#include <iostream>
#include <vector>
#include <chrono>
#include <random>

using namespace std;

const int SIZE = 4096;

// Basic function to access matrix elements
inline int getElement(const std::vector<std::vector<int>>& matrix, int row, int col) {
    return matrix[row][col];
}

// Basic function to add two integers
inline int add(int a, int b) {
    return a + b;
}

// Unoptimized summation function
long long sumMatrixBasic(const std::vector<std::vector<int>>& matrix) {
    long long sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            sum = add(sum, getElement(matrix, i, j));
        }
    }
    return sum;
}

inline int getElement_inl(const vector<vector<int>>& m, int r, int c) noexcept {
    return m[r][c];
}
inline long long add_inl(long long a, int b) noexcept {
    return a + b;
}

long long sumMatrixInlineIndex(const vector<vector<int>>& m) {
    long long sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            sum = add_inl(sum, getElement_inl(m, i, j));
        }
    }
    return sum;
}

long long sumMatrixRowPointer(const vector<vector<int>>& m) {
    long long sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        const int* p   = &m[i][0];
        const int* end = p + SIZE;
        for (; p < end; ++p) {
            sum += *p;
        }
    }
    return sum;
}

long long sumMatrixSinglePointer(const vector<vector<int>>& m) {
    long long sum = 0;
    int row = 0;
    const int* p = &m[row][0];
    const int* endRow = p + SIZE;

    const size_t total = static_cast<size_t>(SIZE) * SIZE;
    for (size_t k = 0; k < total; ++k) {
        sum += *p++;
        if (p == endRow && k + 1 < total) {
            ++row;
            p = &m[row][0];
            endRow = p + SIZE;
        }
    }
    return sum;
}


int main() {
    // Generate a large random matrix
    std::vector<std::vector<int>> matrix(SIZE, std::vector<int>(SIZE));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(-100, 100);
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            matrix[i][j] = distrib(gen);
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    long long sum = sumMatrixBasic(matrix);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Basic Sum: " << sum << std::endl;
    std::cout << "Basic Time: " << duration.count() << " milliseconds" << std::endl;

    // Students will implement their optimized version here
    auto start_optimized = std::chrono::high_resolution_clock::now();
    long long optimized_sum = sumMatrixInlineIndex(matrix); // Students will calculate this
    auto end_optimized = std::chrono::high_resolution_clock::now();
    auto duration_optimized = std::chrono::duration_cast<std::chrono::milliseconds>(end_optimized - start_optimized);

    std::cout << "Optimized Sum: " << optimized_sum << std::endl;
    std::cout << "Optimized Time: " << duration_optimized.count() << " milliseconds" << std::endl;

    auto start_b = std::chrono::high_resolution_clock::now();
    long long b_sum = sumMatrixRowPointer(matrix); // Students will calculate this
    auto end_b = std::chrono::high_resolution_clock::now();
    auto duration_b = std::chrono::duration_cast<std::chrono::milliseconds>(end_b - start_b);

    cout << "RowPointer Sum:  " << b_sum << "\n";
    cout << "RowPointer Time: " << duration_b.count() << " ms\n";

    auto start_c = std::chrono::high_resolution_clock::now();
    long long c_sum = sumMatrixSinglePointer(matrix); // Students will calculate this
    auto end_c = std::chrono::high_resolution_clock::now();
    auto duration_c = std::chrono::duration_cast<std::chrono::milliseconds>(end_c - start_c);

    cout << "SinglePtr Sum:  " << c_sum << "\n";
    cout << "SinglePtr Time: " << duration_c.count() << " ms\n";


    return 0;
}


