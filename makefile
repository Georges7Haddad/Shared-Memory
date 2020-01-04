bench: bench.o
	gcc bench.o -o bench
bench.o: bench.c optimization shm
	gcc -c bench.c
shm: shm.c
	gcc -o shm shm.c
optimization: optimization.c
		gcc -o optimization optimization.c -lpthread
