.SILENT:

all: setup
	rm -f bin/sts
	g++ -std=c++11 -ggdb -Iinclude src/main.cpp -o bin/sts

setup:
	mkdir -p bin

run:
	st -e ./bin/sts
