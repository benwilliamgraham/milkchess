.PHONY: milkchess
milkchess:
	clang -O3 -Wall -Werror -ansi -o milkchess $(wildcard src/*.c)

.PHONY: format
format:
	clang-format -i src/*.c src/*.h
