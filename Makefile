CXX = nvcc

CXXFLAGS = -Xcompiler="-Wall -Werror" -g
NVCCFLAGS = -Xcompiler="-Wall -Werror" -g

TARGET = main

SOURCES = $(shell find . \( -name "*.cpp" -o -name "*.cu" \))
OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.cu=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -x cu -c $< -o $@

%.o: %.cu
	$(CXX) $(NVCCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)