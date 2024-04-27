#include "boost/program_options.hpp"
#include <iostream>
#include <cstring>
#include <cmath>
#include <ctime>

#define CORNER1 10
#define CORNER2 20
#define CORNER3 30
#define CORNER4 20

class Array
{
public:
	Array(int size)
	{
		this->data = new double[size * size];
		this->totalSize = size * size;
		std::memset(this->data, 0, size * size * sizeof(double));

		this->data[0] = CORNER1;
		this->data[size - 1] = CORNER2;
		this->data[size * size - 1] = CORNER3;
		this->data[size * (size - 1)] = CORNER4;

		const double step = 1.0 * (CORNER2 - CORNER1) / (size - 1);
		for (int i = 1; i < size - 1; i++)
		{
			this->data[i] = CORNER1 + i * step;
			this->data[i * size] = CORNER1 + i * step;
			this->data[(size - 1) + i * size] = CORNER2 + i * step;
			this->data[size * (size - 1) + i] = CORNER4 + i * step;
		}

		#pragma acc enter data copyin(this->data[0:totalSize]) async(1)
	}

	~Array()
	{
		#pragma acc update host(this->data[0:totalSize]) async(1)	
		//#pragma acc exit data copyout(this->data[0:totalSize])
		delete[] data;
	}

	double* data;
	size_t totalSize;
};

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

    Array matrixA(size);
	Array matrixB(size);
    const size_t totalSize = size * size;
    
    double error = 1.0;
    int iter = 0;

    std::cout << "Start: " << std::endl;

    #pragma acc enter data copyin(error) async(1)
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

			#pragma acc data present(matrixA.data[0:totalSize], matrixB.data[0:totalSize], error)
			#pragma acc parallel loop independent collapse(2) vector vector_length(256) gang num_gangs(256) reduction(max:error) async(1)
			for (int i = 1; i < size - 1; i++)
			{
				for (int j = 1; j < size - 1; j++)
				{
				matrixB.data[i * size + j] = 0.25 * 
					(matrixA.data[i * size + j - 1] +
					matrixA.data[(i - 1) * size + j] +
					matrixA.data[(i + 1) * size + j] +
					matrixA.data[i * size + j + 1]);

				if (iter % 100 == 0) error = fmax(error, fabs(matrixB.data[i * size + j] - matrixA.data[i * size + j]));
				}
			}

			if(iter % 100 == 0)
			{
			#pragma acc update host(error) async(1)	
			#pragma acc wait(1)
			}

			double* temp = matrixA.data;
			matrixA.data = matrixB.data;
			matrixB.data = temp;
		}

		clock_t end = clock();
		std::cout << "Time: " << 1.0 * (end - begin) / CLOCKS_PER_SEC << std::endl; 
    }
	
    std::cout << "Iter: " << iter << " Error: " << error << std::endl;
	
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			std::cout << matrixA.data[i * size + j] << " ";
		}
		std::cout << "\n";
	}
    return 0;
}