CFLAGS = -Ofast -march=native -mtune=native -std=c++14

main: main.cpp
	g++ -o main $(CFLAGS) main.cpp
