.PHONY: milkchess
milkchess:
	clang++ -O3 -Wall -Werror -o milkchess $(wildcard src/*.cpp)

.PHONY: format
format:
	clang-format -i src/*.cpp src/*.hpp
