#include <cmath>
#include <omp.h>
#include <time.h>
#include <iostream>

const double a = -4.0;
const double b = 4.0;
const int nsteps = 40000000;

double cpuSecond()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

double func(double x)
{
	return exp(-x * x);
}

double integrate_omp(double (*func)(double), double a, double b, int n)
{
	double h = (b - a) / n;
	double sum = 0.0;

#pragma omp parallel num_threads(40)
	{
		int nthreads = omp_get_num_threads();
		int threadid = omp_get_thread_num();
		int items_per_thread = n / nthreads;
		int lb = threadid * items_per_thread;
		int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

		double sumloc = 0.0;

		for (int i = lb; i <= ub; i++)
		{
			sumloc += func(a + h * (i + 0.5));
		}

#pragma omp atomic
		sum += sumloc;
	}

	sum *= h;
	return sum;
}

int main()
{
	double timeStart = cpuSecond();

	integrate_omp(func, a, b, nsteps);

	double timeRes = cpuSecond() - timeStart;

	std::cout << timeRes;
	return 0;
}