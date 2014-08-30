.SILENT:

all: setup
	g++ -std=c++11 -Iinclude src/main.cpp -o bin/sts

setup:
	mkdir -p bin
