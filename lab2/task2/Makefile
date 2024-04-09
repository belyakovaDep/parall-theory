CXX = g++
CXXFLAGS = -std=c++11 -fopenmp
export NSTEPS := -1
export A := -4
export B := 4

CXXFLAGS += -DA=$(A) -DB=$(B) -DNSTEPS=$(NSTEPS)

all:
	$(CXX) $(CXXFLAGS) -o taskTwo task.cpp