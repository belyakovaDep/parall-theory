CXX = g++
CXXFLAGS = -std=c++11 -fopenmp
export T := 0.0001
export E := 0.00001
export N := -1

CXXFLAGS += -DT=$(T) -DE=$(E) -DN=$(N)

all:
	$(CXX) $(CXXFLAGS) -o taskThreeParallel task.cpp