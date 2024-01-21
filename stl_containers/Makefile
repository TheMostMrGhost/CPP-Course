CXX=g++
CXXFLAGS=-Wall -Wextra -O2 -std=c++20
TARGET=parking

all: $(TARGET)

$(TARGET): parking.cc
	$(CXX) $(CXXFLAGS) parking.cc -o $(TARGET)

clean:
	rm -f $(TARGET)
