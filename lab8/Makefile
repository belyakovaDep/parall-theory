CXX = nvcc
CXXFLAGS = -std=c++20 -arch=native
LD := nvcc
LDFLAGS := -lboost_program_options
SOURCES := try.cu
OBJECTS := $(SOURCES:.cpp=.o)
TARGETS = try

%.o:%.cu
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(TARGETS)

try: try.o
	$(LD) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS) $(OBJECTS)