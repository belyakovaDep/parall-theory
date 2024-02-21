#include <iostream>
#include <math.h>
#include <omp.h>
#include <string>
#include <cstring>
void omp_set_num_threads(int num_threads);

double cpuSecond()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void matrix_vector_product_omp(double* a, double* b, double** c, int n)
{
#pragma omp parallel for schedule (static, 45)
	for (int i = 0; i <= n; i++)
	{
		(*c)[i] = 0.0;
		for (int j = 0; j < n; j++)
		{
			(*c)[i] += a[i * n + j] * b[j];
		}
	}
}

void substraction(double* a, double* b, double** c, int n)
{
#pragma omp parallel for schedule (static, 45)
	for (int i = 0; i < n; i++)
	{
		(*c)[i] = a[i] - b[i];
	}
}

void mult(double* a, double t, double** c, int n)
{
#pragma omp parallel for schedule (static, 45)
	for (int i = 0; i < n; i++)
	{
		(*c)[i] = a[i] * t;
	}
}

void iteration_alg(double* matrix, double* x, double* b, double* newX, int n)
{
	matrix_vector_product_omp(matrix, x, &newX, n);
	substraction(newX, b, &newX, n);
	mult(newX, 0.001, &newX, n);
	substraction(x, newX, &newX, n);
}

bool check_alg(double* matrix, double* x, double* b, double* checking, int n)
{
	double check = 0, bcheck = 0, res;
	matrix_vector_product_omp(matrix, x, &checking, n);
	substraction(checking, b, &checking, n);

	for (int i = 0; i < n; i++)
	{
		check += checking[i] * checking[i];
		bcheck += b[i] * b[i];
	}
	res = check / bcheck;

	if (res <= 0.00001) return true;
	return false;
}

int main()
{
	double t = cpuSecond();

	int n = 800;
	double* matrix, * vector, * results, *changed_one;

	matrix = (double*)malloc(sizeof(double) * n * n);
	vector = (double*)malloc(sizeof(double) * n);
	results = (double*)malloc(sizeof(double) * n);
	changed_one = (double*)malloc(sizeof(double) * n);

	for (int i = 0; i < n; i++)
	{
		vector[i] = i + 2;
		results[i] = 0;

		for (int j = 0; j < n; j++)
		{
			matrix[i * n + j] = (i == j) ? 2.0 : 1.0;
		}
	}

	omp_set_num_threads(20);

	while(1)
	{
		iteration_alg(matrix, results, vector, changed_one, n);
		memcpy(results, changed_one, sizeof(double) * n);

		if (check_alg(matrix, results, vector, changed_one, n)) break;
	}

	t = cpuSecond() - t;

	std::cout << t;

	return 0;
}