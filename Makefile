CFLAGS = -g -Wall -Wextra -O3 -std=c99

main :
	mpicc $(CFLAGS) gradient_filters.c -o gradient_filters

run : main
	mpirun -np 1 ./gradient_filters
