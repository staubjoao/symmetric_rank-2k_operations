CC = gcc
MPI = mpicc
CFLAGS = -O0 -w -g
MPIFLAGS = -g -Wall
PATH_SRC = ./src/

all: syr2k syr2k_paralelo syr2k_paralelo_mpi syr2k_pthreads_mpi

syr2k: 
	$(CC) $(CFLAGS) -lm -o syr2k $(PATH_SRC)syr2k.c

syr2k_paralelo: 
	$(CC) $(CFLAGS) -o syr2k_paralelo $(PATH_SRC)syr2k_paralelo.c

syr2k_paralelo_mpi: 
	$(MPI) $(MPIFLAGS) -o syr2k_paralelo_mpi $(PATH_SRC)syr2k_paralelo_mpi.c

syr2k_pthreads_mpi:
	$(MPI) $(MPIFLAGS) -o syr2k_pthreads_mpi $(PATH_SRC)syr2k_pthreads_mpi.c

clear:
	rm -f syr2k syr2k_paralelo syr2k_paralelo_mpi syr2k_pthreads_mpi