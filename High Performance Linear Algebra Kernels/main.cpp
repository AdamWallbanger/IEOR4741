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
    int count = 1;
    double* matrix = new double[row*column];
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            matrix[i * column + j] = count;
            count++;
        }
    }
    return matrix;
}

double* create_vector(int row)
{
    int count = 1;
    double* vector = new double[row];
    for (int i = 0; i < row; i++)
    {
        vector[i] = count;
        count++;
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
    int test_dim[3] = {4,64,256};

    for (int i = 0; i < 3; i++)
    {
        double result_mv[test_dim[i]];
        double result_mm[test_dim[i]*test_dim[i]];
        double* matrix = create_matrix(test_dim[i],test_dim[i]);
        double* vector = create_vector(test_dim[i]);
        double* matrixA = create_matrix(test_dim[i],test_dim[i]);
        double* matrixB = create_matrix(test_dim[i],test_dim[i]);
        Timer timer;
        multiply_mv_row_major(matrix,test_dim[i],test_dim[i],vector,result_mv);
        long long duration = timer.elapsed_microseconds();
        cout <<test_dim[i] << " Size MV Row_major: " << duration << "ms" <<endl;
        // Timer timer1;
        // multiply_mv_col_major(matrix,test_dim[i],test_dim[i],vector,result);
        // long long duration1 = timer1.elapsed_microseconds();
        // cout <<test_dim[i] << " Size MV Col_major: " << duration1 << "ms" <<endl;
        delete[] matrix;
        delete[] vector;
        Timer timer2;
        multiply_mm_naive(matrixA,test_dim[i],test_dim[i],matrixB,test_dim[i],test_dim[i],result_mm);
        long long duration2 = timer2.elapsed_microseconds();
        cout <<test_dim[i] << " Size MM Naive: " << duration2 << "ms" <<endl;
        Timer timer3;
        multiply_mm_transposed_b(matrixA,test_dim[i],test_dim[i],matrixB,test_dim[i],test_dim[i],result_mm);
        long long duration3 = timer3.elapsed_microseconds();
        cout <<test_dim[i] << " Size MV Transposed: " << duration3 << "ms" <<endl;
        delete[] matrixA;
        delete[] matrixB;
    }

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
