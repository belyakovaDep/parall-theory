#include <iostream>
#include <cstring>
#include <cmath>
#include <ctime>
#include "boost/program_options.hpp"

#include <cuda_runtime.h>
#include <cub/cub.cuh>

namespace boo = boost::program_options;

struct CudaDeleter {
    void operator()(double* ptr) const {
        cudaFree(ptr);
    }
};

struct CudaStreamDeleter {
    void operator()(cudaStream_t* stream) const {
        cudaStreamDestroy(*stream);
    }
};

struct CudaGraphDeleter {
    void operator()(cudaGraph_t* graph) const {
        cudaGraphDestroy(*graph);
    }
};

std::unique_ptr<cudaStream_t, CudaStreamDeleter> createCudaStream() {
    cudaStream_t* stream = new cudaStream_t;
    cudaError_t err = cudaStreamCreate(stream);
    return std::unique_ptr<cudaStream_t, CudaStreamDeleter>(stream);
}

std::unique_ptr<cudaGraph_t, CudaGraphDeleter> createCudaGraph() {
    cudaGraph_t* graph = new cudaGraph_t;
    return std::unique_ptr<cudaGraph_t, CudaGraphDeleter>(graph);
}

__global__
void calculateMatrix(double* matrixA, double* matrixB, size_t size)
{
	unsigned int j = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int i = blockIdx.y * blockDim.y + threadIdx.y;

	if (i * size + j > size * size) return;
	
	if(!((j == 0 || i == 0 || j == size - 1 || i == size - 1)))
	{
		matrixB[i * size + j] = 0.25 * (matrixA[i * size + j - 1] + matrixA[(i - 1) * size + j] +
							matrixA[(i + 1) * size + j] + matrixA[i * size + j + 1]);		
	}
}

__global__
void getErrorMatrix(double* matrixA, double* matrixB, double* outputMatrix, size_t size)
{
	size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx > size * size) return;

	outputMatrix[idx] = std::abs(matrixB[idx] - matrixA[idx]);
}

const double corner1 = 10;
const double corner2 = 20;
const double corner3 = 30;
const double corner4 = 20;

const int error_step = 100;

int main(int argc, char** argv)
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
	
	std::unique_ptr<double, CudaDeleter> uMatrixA;
	double *matrixA = uMatrixA.get();

	std::unique_ptr<double, CudaDeleter> uMatrixB;
	double *matrixB = uMatrixB.get();

	cudaMallocHost(&matrixA, totalSize * sizeof(double));
	cudaMallocHost(&matrixB, totalSize * sizeof(double));
	
	std::memset(matrixA, 0, totalSize * sizeof(double));

	matrixA[0] = corner1;
	matrixA[size - 1] = corner2;
	matrixA[size * size - 1] = corner3;
	matrixA[size * (size - 1)] = corner4;

	const double step = 1.0 * (corner2 - corner1) / (size - 1);
	for (int i = 1; i < size - 1; i++)
	{
		matrixA[i] = corner1 + i * step;
		matrixA[i * size] = corner1 + i * step;
		matrixA[size - 1 + i * size] = corner2 + i * step;
		matrixA[size * (size - 1) + i] = corner4 + i * step;
	}

	std::memcpy(matrixB, matrixA, totalSize * sizeof(double));

	size_t tempStorageSize = 0;

	std::unique_ptr<double, CudaDeleter> uDeviceMatrixAPtr;
	double *deviceMatrixAPtr = uDeviceMatrixAPtr.get();

	std::unique_ptr<double, CudaDeleter> uDeviceMatrixBPtr;
	double *deviceMatrixBPtr = uDeviceMatrixBPtr.get();

	std::unique_ptr<double, CudaDeleter> uDeviceError;
	double *deviceError = uDeviceError.get();

	std::unique_ptr<double, CudaDeleter> uErrorMatrix;
	double *errorMatrix = uErrorMatrix.get();

	std::unique_ptr<double, CudaDeleter> uTempStorage;
	double *tempStorage = uTempStorage.get();

	cudaError_t cudaStatus_1 = cudaMalloc((void**)(&deviceMatrixAPtr), sizeof(double) * totalSize);
	cudaError_t cudaStatus_2 = cudaMalloc((void**)(&deviceMatrixBPtr), sizeof(double) * totalSize);
	cudaMalloc((void**)&deviceError, sizeof(double));
	cudaStatus_1 = cudaMalloc((void**)&errorMatrix, sizeof(double) * totalSize);

	
	if (cudaStatus_1 != 0 || cudaStatus_2 != 0)
	{
		std::cout << "Memory allocation error" << std::endl;
		return -1;
	}

	cudaStatus_1 = cudaMemcpy(deviceMatrixAPtr, matrixA, sizeof(double) * totalSize, cudaMemcpyHostToDevice);
	cudaStatus_2 = cudaMemcpy(deviceMatrixBPtr, matrixB, sizeof(double) * totalSize, cudaMemcpyHostToDevice);

	if (cudaStatus_1 != 0 || cudaStatus_2 != 0)
	{
		std::cout << "Memory transfering error" << std::endl;
		return -1;
	}

	cub::DeviceReduce::Max(tempStorage, tempStorageSize, errorMatrix, deviceError, totalSize);
	
	cudaMalloc((void**)&tempStorage, tempStorageSize);

	int iter = 0; 
	std::unique_ptr<double, CudaDeleter> uError;
	double *error = uError.get();
	cudaMallocHost(&error, sizeof(double));
	*error = 1.0;

	auto uStream = createCudaStream();
    cudaStream_t stream = *uStream.get();

	bool isGraphCreated = false;
	//cudaStream_t stream;
	auto uGraph = createCudaGraph();
	auto graph = uGraph.get();
	cudaGraphExec_t instance;

	size_t threads = (size < 1024) ? size : 1024;
    unsigned int blocks = size / threads;

	dim3 blockDim(32, 32);
    dim3 gridDim((size + blockDim.x - 1) /  blockDim.x, (size + blockDim.y - 1) /  blockDim.y);

    std::cout << "Start: " << std::endl;

	clock_t begin = clock();
	while(iter < maxIter && *error > minError)
	{
		if (isGraphCreated)
		{
			cudaGraphLaunch(instance, stream);
			
			cudaMemcpyAsync(error, deviceError, sizeof(double), cudaMemcpyDeviceToHost, stream);

			cudaStreamSynchronize(stream);

			iter += error_step;
		}
		else
		{
			cudaStreamBeginCapture(stream, cudaStreamCaptureModeGlobal);
			for(size_t i = 0; i < error_step / 2; i++)
			{
				calculateMatrix<<<gridDim, blockDim, 0, stream>>>(deviceMatrixAPtr, deviceMatrixBPtr, size);
				calculateMatrix<<<gridDim, blockDim, 0, stream>>>(deviceMatrixBPtr, deviceMatrixAPtr, size);
			}

			getErrorMatrix<<<threads * blocks * blocks, threads,  0, stream>>>(deviceMatrixAPtr, deviceMatrixBPtr, errorMatrix, size);
			cub::DeviceReduce::Max(tempStorage, tempStorageSize, errorMatrix, deviceError, totalSize, stream);
	
			cudaStreamEndCapture(stream, graph);
			cudaGraphInstantiate(&instance, *graph, NULL, NULL, 0);
			isGraphCreated = true;
  		}
	}

	clock_t end = clock();
	std::cout << "Time: " << 1.0 * (end - begin) / CLOCKS_PER_SEC << std::endl;
	std::cout << "Iter: " << iter << " Error: " << *error << std::endl;

	return 0;
}