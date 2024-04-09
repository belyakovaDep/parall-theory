#include <iostream>
#include <cmath>
#include <omp.h>
#include <string>
#include <cstring>
#include <memory>

void omp_set_num_threads(int num_threads);

double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void matrix_vector_product_omp(double* a, double* b, double* c, int n)
{
#pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
        {
            c[i] += a[i * n + j] * b[j];
        }
    }
}

void substraction(double* a, double* b, double* c, int n)
{
#pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        c[i] = a[i] - b[i];
    }
}

void mult(double* a, double t, double* c, int n)
{
#pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        c[i] = a[i] * t;
    }
}

void iteration_alg(double* matrix, double* x, double* b, double* newX, int n)
{
    matrix_vector_product_omp(matrix, x, newX, n);
    substraction(newX, b, newX, n);
    mult(newX, T, newX, n);
    substraction(x, newX, newX, n);
}

bool check_alg(double* matrix, double* x, double* b, double* checking, int n)
{
    double check = 0, bcheck = 0, res;
    matrix_vector_product_omp(matrix, x, checking, n);
    substraction(checking, b, checking, n);

    for (int i = 0; i < n; i++)
    {
        check += checking[i] * checking[i];
        bcheck += b[i] * b[i];
    }
    res = check / bcheck;

    if (res <= E) return true;
    return false;
}

int main()
{
    if (N <= 0)
    {
        throw std::length_error("Wrong size of matrix!");
    }
    double t = cpuSecond();

    int n = N;
    std::unique_ptr<double[]> matrix(new double[n * n]);
    std::unique_ptr<double[]> vector(new double[n]);
    std::unique_ptr<double[]> results(new double[n]);
    std::unique_ptr<double[]> changed_one(new double[n]);

    for (int i = 0; i < n; i++)
    {
        vector[i] = n + 1;
        results[i] = 0;

        for (int j = 0; j < n; j++)
        {
            matrix[i * n + j] = (i == j) ? 2.0 : 1.0;
        }
    }

    omp_set_num_threads(20);

    while(!check_alg(matrix.get(), results.get(), vector.get(), changed_one.get(), n))
    {
        iteration_alg(matrix.get(), results.get(), vector.get(), changed_one.get(), n);
        std::memcpy(results.get(), changed_one.get(), sizeof(double) * n);
    }

    t = cpuSecond() - t;

    std::cout << t << std::endl;

    return 0;
}