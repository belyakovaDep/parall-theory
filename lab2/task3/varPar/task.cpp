#include <iostream>
#include <omp.h>
#include <cstring>

double cpuSecond()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void integration(const double* matrix, const double* b, const double* x, double** res, int n)
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
        (*res)[i] = 0.0;

			for (int j = 0; j < n; j++)
			{
				(*res)[i] += matrix[i * n + j] * x[j];
			}

    }

    for (int i = lb; i <= ub; i++)
	{
		(*res)[i] = (*res)[i] - b[i];
	}

    for (int i = lb; i <= ub; i++)
	{
		(*res)[i] = (*res)[i] * (0.0001);
	}

	for (int i = lb; i <= ub; i++)
	{
		(*res)[i] = x[i] - (*res)[i];
	}
}
}

void cycleChecking(const double* matrix, const double* b, double** x, double** res, int n)
{
	double* check = (double*)malloc(sizeof(double) * n);
	double misUp = 0.0, misDown = 0.0, mis;

	while(1)
	{
		integration(matrix, b, *x, res, n);

		misUp = 0.0; misDown = 0.0;

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
					check[i] += matrix[i * n + j] * (*x)[j];
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
				misDown += localsumB;
		}

		mis = misUp/misDown;
		if(mis <= 0.00001) break;

		memcpy(*x, *res, sizeof(double) * n);
	}
}

int main()
{
	int n = 17000;
	double* matrix, * vector, * results, *changed_one;

	matrix = (double*)malloc(sizeof(double) * n * n);
	vector = (double*)malloc(sizeof(double) * n);
	results = (double*)malloc(sizeof(double) * n);
	changed_one = (double*)malloc(sizeof(double) * n);

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

    cycleChecking(matrix, vector, &results, &changed_one, n);

	t = cpuSecond() - t;

	std::cout << t << std::endl;
	std::cout<< changed_one[0] << " " << changed_one[1] << std::endl;
    return 0;
}