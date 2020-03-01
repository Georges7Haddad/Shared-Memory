bench:
	gcc -o bench bench.c
	gcc -o shm shm.c
	gcc -o optimization optimization.c -lpthread
shm:	shm
	gcc -o shm shm.c
optimization:	optimization
	gcc -o optimization optimization.c -lpthread
