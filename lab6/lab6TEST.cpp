#include "boost/program_options.hpp"
#include <iostream>
#include <cstring>
#include <cmath>
#include <ctime>

#define CORNER1 10
#define CORNER2 20
#define CORNER3 30
#define CORNER4 20

namespace boo = boost::program_options;
int main(int argc, char* argv[])
{
    boo::options_description desc{"Options"};
    desc.add_options()
      ("help,h", "help screen")
      ("accuracy", boo::value<float>()->default_value(0.000001), "the least avaliable error value")
      ("size", boo::value<int>()->default_value(10), "size of the matrix")
      ("iterations", boo::value<int>()->default_value(50), "the most avaliable number of iterations");

    boo::variables_map vm;
    boo::store(parse_command_line(argc, argv, desc), vm);
    boo::notify(vm);

    if (vm.count("help")) std::cout << desc << '\n';
    const size_t size = vm["size"].as<int>();
    const size_t maxIter = vm["iterations"].as<int>();
    const float minError = vm["accuracy"].as<float>();

    
    const size_t totalSize = size * size;
    double* matrixA = new double[size * size];
	double* matrixB = new double[size * size];
    std::memset(matrixA, 0, size * size * sizeof(double));

    matrixA[0] = CORNER1;
	matrixA[size - 1] = CORNER2;
	matrixA[size * size - 1] = CORNER3;
	matrixA[size * (size - 1)] = CORNER4;

    const double step = 1.0 * (CORNER2 - CORNER1) / (size - 1);
	for (int i = 1; i < size - 1; i++)
	{
		matrixA[i] = CORNER1 + i * step;
		matrixA[i * size] = CORNER1 + i * step;
		matrixA[(size - 1) + i * size] = CORNER2 + i * step;
		matrixA[size * (size - 1) + i] = CORNER4 + i * step;
	}
	std::memcpy(matrixB, matrixA, size * size * sizeof(double));

    
    double error = 1.0;
    int iter = 0;

    std::cout << "Start: " << std::endl;

    #pragma acc enter data copyin(matrixA[0:totalSize], matrixB[0:totalSize], error)
    {
		clock_t begin = clock();
		while (error > minError && iter < maxIter)
		{	    
			iter++;
			
			if(iter % 100 == 0)
			{
			error = 0.0;
			#pragma acc update device(error) async(1)
			}

			#pragma acc data present(matrixA, matrixB, error)
			#pragma acc parallel loop independent collapse(2) vector vector_length(256) gang num_gangs(256) reduction(max:error) async(1)
			for (int i = 1; i < size - 1; i++)
			{
				for (int j = 1; j < size - 1; j++)
				{
				matrixB[i * size + j] = 0.25 * 
					(matrixA[i * size + j - 1] +
					matrixA[(i - 1) * size + j] +
					matrixA[(i + 1) * size + j] +
					matrixA[i * size + j + 1]);

				error = fmax(error, fabs(matrixB[i * size + j] - matrixA[i * size + j]));
				}
			}

			if(iter % 100 == 0)
			{
			#pragma acc update host(error) async(1)	
			#pragma acc wait(1)
			}

			double* temp = matrixA;
			matrixA = matrixB;
			matrixB = temp;
		}

		clock_t end = clock();
		std::cout << "Time: " << 1.0 * (end - begin) / CLOCKS_PER_SEC << std::endl; 
		#pragma acc update host(matrixA[0:totalSize], matrixB[0:totalSize])	
    }
	
    std::cout << "Iter: " << iter << " Error: " << error << std::endl;
	
	// for (int i = 0; i < size; i++)
	// {
	// 	for (int j = 0; j < size; j++)
	// 	{
	// 		std::cout << matrixA[i * size + j] << " ";
	// 	}
	// 	std::cout << "\n";
	// }
    delete[] matrixA;
    delete[] matrixB;
    return 0;
}