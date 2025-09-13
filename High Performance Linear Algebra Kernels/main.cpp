#include <iostream>
#include <chrono>
#include <cassert>
using namespace std;

void multiply_mv_row_major(const double* matrix, int rows, int cols, const double* vector, double* result);
void multiply_mv_col_major(const double* matrix, int rows, int cols, const double* vector, double* result);
void multiply_mm_naive(const double* matrixA, int rowsA, int colsA, const double* matrixB, int rowsB, int colsB, double* result);
void multiply_mm_transposed_b(const double* matrixA, int rowsA, int colsA, const double* matrixB_transposed, int rowsB, int colsB, double* result);

double* create_matrix(int row,int column)
{
    double* matrix = new double[row*column];
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            matrix[i * column + j] = 2;
        }
    }
    return matrix;
}

double* create_vector(int row)
{
    double* vector = new double[row];
    for (int i = 0; i < row; i++)
    {
        vector[i] = 2;
    }
    return vector;
}

class Timer
{
    public:
        Timer() : start_(std::chrono::high_resolution_clock::now()) {}
        long long elapsed_microseconds() const
        {
            auto end_ = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
        }

    private:
        std::chrono::high_resolution_clock::time_point start_;
};

int main()
{
    int row,column;
    row = 3;
    column = 2;
    double* matrix = create_matrix(row,column);
    double* vector = create_vector(column);
    double* matrixA = create_matrix(row,column);
    double* matrixB = create_matrix(row,column);
    double result[column];
    delete[] matrix;
    delete[] vector;
    Timer timer;
    long long duration = timer.elapsed_microseconds();
    cout << duration << endl;
    return 0;
}

void multiply_mv_row_major(const double* matrix, int rows, int cols, const double* vector, double* result)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            result[i] += matrix[i * cols + j] * vector[j];
        }
    }
}
void multiply_mm_naive(const double* matrixA, int rowsA, int colsA, const double* matrixB, int rowsB, int colsB, double* result)
{
    assert(colsA == rowsB);
    assert(matrixA != nullptr);
    assert(matrixB != nullptr);
    assert(result != nullptr);
    for (int i = 0; i < rowsA; i++)
    {
        for (int j = 0; j < colsA; j++)
        {
            double value_A = matrixA[i * colsA + j];
            for (int k = 0; k < colsB; k++)
            {
                result[i * colsA + j] += value_A * matrixB[k * colsB + j];
            }
        }
    }
}
void multiply_mm_transposed_b(const double* matrixA, int rowsA, int colsA, const double* matrixB_transposed, int rowsB, int colsB, double* result)
{
    //这里我没搞懂，如果matrixB_transposed传进来之前就已经是transposde过了，那直接正常乘就行了。暂时写了现场transposed
    assert(colsA == colsB);
    assert(matrixA != nullptr);
    assert(matrixB_transposed != nullptr);
    assert(result != nullptr);
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < rowsB; ++j) {
            double sum = 0;
            for (int k = 0; k < colsA; ++k) {
                double value_A = matrixA[i * colsA + k];
                double value_B = matrixB_transposed[j * colsB + k];
                sum += value_A * value_B;
            }
            result[i * rowsB + j] = sum;
        }
    }
}
