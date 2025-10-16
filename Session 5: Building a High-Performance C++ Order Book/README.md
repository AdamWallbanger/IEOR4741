Put the codes in the file contains CMakeList.txt
cd the file
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/benchmark_vector
./build/benchmark_map
./build/benchmark_heaps
in the terminal just type these
