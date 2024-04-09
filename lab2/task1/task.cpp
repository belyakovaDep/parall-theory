#include <omp.h>
#include <iostream>
#include <time.h>

double cpuSecond()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void matrix_vector_product_omp(double* a, double* b, double* c, int n)
{
#pragma omp parallel num_threads(16)
	{
		int nthreads = omp_get_num_threads();
		int threadid = omp_get_thread_num();
		int items_per_thread = n / nthreads;
		int lb = threadid * items_per_thread;
		int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

		for (int i = lb; i <= ub; i++) 
		{
			c[i] = 0.0;

			for (int j = 0; j < n; j++)
			{
				c[i] += a[i * n + j] * b[j];
			}

		}
	}
}

void run_parallel(int n)
{
	double* matrix, *vector, *output;

	matrix = (double*)malloc(sizeof(double) * n * n);
	vector = (double*)malloc(sizeof(double) * n);
	output = (double*)malloc(sizeof(double) * n);

	for (int i = 0; i < n; i++) 
	{
		for (int j = 0; j < n; j++)
		{
			matrix[i * n + j] = ((double)i + (double)j) * 0.001;
		}
	}

	for (int j = 0; j < n; j++)
	{
		vector[j] = (double)j * 0.001;
	}

	double t = cpuSecond();
	matrix_vector_product_omp(matrix, vector, output, n);
	t = cpuSecond() - t;

	std::cout << "Elapsed time (parallel): " << t << std::endl;;

	free(matrix);
	free(vector);
	free(output);
}

int main(int argc, char** argv)
{
	if (ARRAYSIZE <= 0) 
	{
		throw std::length_error("Wrong size of array");
	}

	run_parallel(ARRAYSIZE);

	return 0;
}