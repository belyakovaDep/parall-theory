CXX = g++
CXXFLAGS = -std=c++11 -fopenmp

export ARRAYSIZE := -1

CXXFLAGS += -DARRAYSIZE=$(ARRAYSIZE)

all:
	$(CXX) $(CXXFLAGS) -o taskOne task.cpp