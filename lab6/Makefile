CXX = pgc++
CXXFLAGS = -std=c++17 -acc=gpu -Minfo=all
LD := pgc++
LDFLAGS := -lboost_program_options -acc=gpu
SOURCES := lab6TESTcopy.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGETS = lab6TESTcopy

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(TARGETS)

lab6TESTcopy: lab6TESTcopy.o
	$(LD) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS) $(OBJECTS)

# CXX = pgc++
# CXXFLAGS = -std=c++17 -acc=gpu -Minfo=all
# LD := pgc++
# LDFLAGS := -lboost_program_options -acc=gpu
# SOURCES := lab6TEST.cpp
# OBJECTS := $(SOURCES:.cpp=.o)
# TARGETS = lab6TEST

# %.o:%.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# all: $(TARGETS)

# lab6TEST: lab6TEST.o
# 	$(LD) -o $@ $< $(LDFLAGS)

# clean:
# 	rm -f $(TARGETS) $(OBJECTS)