CXX=g++
CXXFLAGS=-std=c++17 -O2 -Wall -Wextra -Werror -g
OUTDIR=bin

# Get all .cpp files in the current directory
SOURCES := $(wildcard *.cpp)
# Generate target names by removing .cpp extension and adding output directory
TARGETS := $(addprefix $(OUTDIR)/, $(SOURCES:.cpp=))

all: $(OUTDIR) $(TARGETS)

$(OUTDIR):
	mkdir -p $(OUTDIR)

# Pattern rule to compile each .cpp file into its corresponding executable
$(OUTDIR)/%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf $(OUTDIR)