CXX      := g++
CXXFLAGS := -O3 -std=c++17 -pthread -g
TARGET   := sim
SRC      := main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) orders.csv
