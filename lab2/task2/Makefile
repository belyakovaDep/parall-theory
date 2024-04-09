CXX = g++
CXXFLAGS = -std=c++11 -fopenmp

ifdef A
	CXXFLAGS += -DA
endif

ifdef B
	CXXFLAGS += -DB
endif

ifdef NSTEPS
	CXXFLAGS += -DNSTEPS
endif


all:
	$(CXX) $(CXXFLAGS) -o taskTwo task.cpp