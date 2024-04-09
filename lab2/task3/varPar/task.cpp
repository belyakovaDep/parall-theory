#include <iostream>
#include <omp.h>
#include <cstring>
#include <memory>

double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void integration(const double* matrix, const double* b, const double* x, double* res, int n)
{
#pragma omp parallel
{
    int nthreads = omp_get_num_threads();
    int threadid = omp_get_thread_num();
    int items_per_thread = n / nthreads;
    int lb = threadid * items_per_thread;
    int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

    for(int i = lb; i <= ub; i++)
    {
        res[i] = 0.0;

        for (int j = 0; j < n; j++)
        {
            res[i] += matrix[i * n + j] * x[j];
        }
    }

    for (int i = lb; i <= ub; i++)
    {
        res[i] = res[i] - b[i];
    }

    for (int i = lb; i <= ub; i++)
    {
        res[i] = res[i] * (T);
    }

    for (int i = lb; i <= ub; i++)
    {
        res[i] = x[i] - res[i];
    }
}
}

void cycleChecking(const double* matrix, const double* b, double* x, double* res, int n)
{
    std::unique_ptr<double[]> check(new double[n]);
    double misUp = 0.0, misDown = 0.0, mis = 1;

    while(mis > E)
    {
        integration(matrix, b, x, res, n);

        misUp = 0.0;
        misDown = 0.0;

#pragma omp parallel
        {
            int nthreads = omp_get_num_threads();
            int threadid = omp_get_thread_num();
            int items_per_thread = n / nthreads;
            int lb = threadid * items_per_thread;
            int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

            double localsum = 0.0, localsumB = 0.0;

            for(int i = lb; i <= ub; i++)
            {
                check[i] = 0.0;

                for (int j = 0; j < n; j++)
                {
                    check[i] += matrix[i * n + j] * x[j];
                }
            }

            for (int i = lb; i <= ub; i++)
            {
                check[i] = check[i] - b[i];
                localsum += check[i] * check[i];
                localsumB += b[i] * b[i];
            }

#pragma omp atomic
            misUp += localsum;
#pragma omp atomic
            misDown += localsumB;
        }

        mis = misUp/misDown;

        std::memcpy(x, res, sizeof(double) * n);
    }
}

int main()
{
    if(N <= 0)
    {
        throw std::length_error("Wrong size of matrix!");
    }
    
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

    omp_set_num_threads(80);

    double t = cpuSecond();

    cycleChecking(matrix.get(), vector.get(), results.get(), changed_one.get(), n);

    t = cpuSecond() - t;

    std::cout << t << std::endl;
    return 0;
}