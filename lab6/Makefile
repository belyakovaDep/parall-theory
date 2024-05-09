CXX = pgc++
CXXFLAGS = -std=c++17 -acc=gpu -Minfo=all
LD := pgc++
LDFLAGS := -lboost_program_options -acc=gpu
SOURCES := lastone.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGETS = lastone

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(TARGETS)

lastone: lastone.o
	$(LD) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS) $(OBJECTS)