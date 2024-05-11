#include <iostream>
#include <thread>
#include <vector>

double cpuSecond()
{
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void mult(double* matrix, double* vector, double* result,
          int n, int startRow, int endRow) 
{
    for (int i = startRow; i < endRow; ++i) 
    {
        double sum = 0;

        for (int j = 0; j < n; ++j)  sum += matrix[i * n + j] * vector[j];
        result[i] = sum;
    }
}

double* multParallel(double* matrix, double* vector, int n)
{
    double* result = new double[n];

    int numThreads = 2;
    std::vector<std::thread> threads(numThreads);

    int chunkSize = n / numThreads;
    int startRow = 0;

    for (unsigned int i = 0; i < numThreads; ++i) 
    {
        int endRow = (i == numThreads - 1) ? n : startRow + chunkSize;
        threads[i] = std::thread(mult, matrix, vector, result, 
                                 n, startRow, endRow);
        startRow = endRow;
    }

    for (auto& t : threads) 
    {
        t.join();
    }

    return result;
}

void fillData(double* matrix, double* vector, int n)
{
    for(int i = 0; i < n; i++)
    {
        vector[i] = i + 1;
        for(int j = 0; j < n; j++) matrix[i * n + j] = i * n + j + 1;
    }
}

int main() 
{
    int n = 20000;

    std::unique_ptr<double[]> matrix(new double[n * n]);
    std::unique_ptr<double[]> vector(new double[n]);

    fillData(matrix.get(), vector.get(), n);

    double t = cpuSecond();

    double* result = multParallel(matrix.get(), vector.get(), n);

    t = cpuSecond() - t;
    std::cout << t << std::endl;

    delete[] result;
    return 0;
}