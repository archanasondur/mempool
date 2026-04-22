CXX      = clang++
CXXFLAGS = -std=c++17
SRC      = pool.cpp main.cpp

debug:
	$(CXX) $(CXXFLAGS) -fsanitize=undefined -g -o mempool_debug $(SRC)

bench:
	$(CXX) $(CXXFLAGS) -O2 -DNDEBUG -o mempool_bench $(SRC)

clean:
	rm -f mempool_debug mempool_bench