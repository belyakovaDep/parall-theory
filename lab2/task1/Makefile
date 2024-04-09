CXX = g++
CXXFLAGS = -std=c++11 -fopenmp

ifdef ARRAYSIZE
	CXXFLAGS += -DARRAYSIZE
endif

all:
	$(CXX) $(CXXFLAGS) -o taskOne task.cpp