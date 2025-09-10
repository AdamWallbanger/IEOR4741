//
// Created by tianci chen on 9/10/25.
//

#ifndef PROGRAMMING_FOR_FE_FUNCTIONS_H
#define PROGRAMMING_FOR_FE_FUNCTIONS_H
#include "functions.h"
void multiply_mv_row_major(const double* matrix, int rows, int cols, const double* vector, double* result);
void multiply_mv_col_major(const double* matrix, int rows, int cols, const double* vector, double* result);

#endif //PROGRAMMING_FOR_FE_FUNCTIONS_H
