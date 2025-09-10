#include <iostream>
#include <chrono>
#include <vector>
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
    double result[column];
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            cout << matrix[i*column+j] << " ";
        }
        cout << endl;
    }
    cout << endl;
    for (int i = 0; i < row; i++)
    {
        cout << result[i] << " ";
    }
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