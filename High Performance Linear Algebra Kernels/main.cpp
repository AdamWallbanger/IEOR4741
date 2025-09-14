#include <iostream>
#include <chrono>
#include <cassert>
#include <cmath>
using namespace std;

void multiply_mv_row_major(const double* matrix, int rows, int cols, const double* vector, double* result);
void multiply_mv_col_major(const double* matrix, int rows, int cols, const double* vector, double* result);
void multiply_mm_naive(const double* matrixA, int rowsA, int colsA, const double* matrixB, int rowsB, int colsB, double* result);
void multiply_mm_transposed_b(const double* matrixA, int rowsA, int colsA, const double* matrixB_transposed, int rowsB, int colsB, double* result);

void average_time(int* time,int size,double& mean,double& std)
{
    double sum = 0,diff = 0;
    for (int i = 0; i < size; i++)
    {
        sum += time[i];
    }
    mean = sum / size;
    for (int i = 0; i < size; i++)
    {
        diff += (time[i] - mean) * (time[i] - mean);
    }
    std = sqrt(diff/size);
}

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
    Timer timer0;
    int test_dim[3] = {4,64,256};
    for (int i = 0; i < 3; i++)
    {
        int time_mv_row[10] = {0};
        int time_mv_col [10] = {0};
        int time_mm_navi[10] = {0};
        int time_mm_tran[10] = {0};
        for (int j = 0; j < 10;j++)
        {
            Timer timer_init;
            double result_mv[test_dim[i]];
            double result_mm[test_dim[i]*test_dim[i]];
            double* matrix = create_matrix(test_dim[i],test_dim[i]);
            double* vector = create_vector(test_dim[i]);
            double* matrixA = create_matrix(test_dim[i],test_dim[i]);
            double* matrixB = create_matrix(test_dim[i],test_dim[i]);
            long long duration_init = timer_init.elapsed_microseconds();
            cout << duration_init << "ms"<<endl;
            Timer timer;
            multiply_mv_row_major(matrix,test_dim[i],test_dim[i],vector,result_mv);
            long long duration = timer.elapsed_microseconds();
            time_mv_row[j] = duration;
            Timer timer1;
            multiply_mv_col_major(matrix,test_dim[i],test_dim[i],vector,result_mv);
            long long duration1 = timer1.elapsed_microseconds();
            time_mv_col[j] = duration1;
            delete[] matrix;
            delete[] vector;
            Timer timer2;
            multiply_mm_naive(matrixA,test_dim[i],test_dim[i],matrixB,test_dim[i],test_dim[i],result_mm);
            long long duration2 = timer2.elapsed_microseconds();
            time_mm_navi[j] = duration2;
            Timer timer3;
            multiply_mm_transposed_b(matrixA,test_dim[i],test_dim[i],matrixB,test_dim[i],test_dim[i],result_mm);
            long long duration3 = timer3.elapsed_microseconds();
            time_mm_tran[j] = duration3;
            delete[] matrixA;
            delete[] matrixB;
        }
        double mean,std;
        average_time(time_mv_row,10,mean,std);
        cout <<"Average time for " <<test_dim[i] << " Size MV Row_major: " << mean << "ms, with std of " << std << endl;
        average_time(time_mv_col,10,mean,std);
        cout <<"Average time for " <<test_dim[i] << " Size MV Col_major: " << mean << "ms, with std of " << std << endl;
        average_time(time_mm_navi,10,mean,std);
        cout <<"Average time for " <<test_dim[i] << " Size MM Naive: " << mean << "ms, with std of " << std  << endl;
        average_time(time_mm_tran,10,mean,std);
        cout <<"Average time for " <<test_dim[i] << " Size MM Transposed: " << mean << "ms, with std of " << std << endl;
        cout << endl;
    }
    long long duration0 = timer0.elapsed_microseconds();
    cout << "Total Time: " << duration0 << "ms" << endl;
    return 0;
}

void multiply_mv_row_major(const double* matrix, int rows, int cols, const double* vector, double* result){
    assert(matrix != nullptr);
    assert(vector != nullptr);
    assert(result != nullptr);
    assert(rows > 0 && cols > 0);
    if (cols != rows) {
        std :: cerr << "Error: cols must equal rows!";
        return;
    }
    for (int i = 0; i < rows; ++i){
        double sum = 0.0;
        const double* row_ptr = matrix + (size_t)i * cols;
        for (int j = 0; j < cols; ++j){
            sum += row_ptr[j] * vector[j];
        }
        result[i] = sum;
    }
}

void multiply_mv_col_major(const double* matrix, int rows, int cols, const double* vector, double* result){
    assert(matrix != nullptr);
    assert(vector != nullptr);
    assert(result != nullptr);
    assert(rows > 0 && cols > 0);
    if (cols != rows) {
        std :: cerr << "Error: cols must equal rows!";
        return;
    }
    for(int i = 0; i < rows; ++i){
        double sum = 0.0;
        for(int j = 0; j < cols; ++j){
            sum += matrix[(size_t)j*rows + i ] * vector[j];
        }
        result[i] = sum;
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
