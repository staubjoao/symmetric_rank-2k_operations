CC = gcc
MPI = mpic++
CFLAGS = -O0 -w -g

all: syr2k syr2k_paralelo syr2k_paralelo_mpi 

syr2k: linear-algebra/kernels/syr2k/syr2k.c
	$(CC) $(CFLAGS) -o syr2k linear-algebra/kernels/syr2k/syr2k.c

syr2k_paralelo: syr2k_paralelo.c
	$(CC) $(CFLAGS) -o syr2k_paralelo syr2k_paralelo.c

syr2k_paralelo_mpi: syr2k_paralelo_mpi.c
	$(MPI) -o syr2k_paralelo_mpi.exe syr2k_paralelo_mpi.c

clean:
	rm -f syr2k syr2k_paralelo
