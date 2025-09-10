//
// Created by tianci chen on 9/10/25.
//
#include<iostream>
#include<cassert>
#include "functions.h"
void multiply_mv_row_major(const double* matrix, int rows, int cols, const double* vector, double* result){
    assert(matrix != nullptr);
    assert(vector != nullptr);
    assert(result != nullptr);
    assert(rows > 0 && cols > 0);
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
    for(int i = 0; i < rows; ++i){
        double sum = 0.0;
        for(int j = 0; j < cols; ++j){
            sum += matrix[(size_t)j*rows + i ] * vector[j];
        }
        result[i] = sum;
    }
}