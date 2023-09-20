CC = gcc
MPI = mpicc
CFLAGS = -O0 -w -g
PAPI = -lpapi
MPIFLAGS = -g -Wall

all: syr2k syr2k_paralelo syr2k_paralelo_mpi 

syr2k: linear-algebra/kernels/syr2k/syr2k.c
	$(CC) $(CFLAGS) -lm -lnetcdf -o syr2k linear-algebra/kernels/syr2k/syr2k.c $(PAPI)

syr2k_paralelo: syr2k_paralelo.c
	$(CC) $(CFLAGS) -o syr2k_paralelo syr2k_paralelo.c

syr2k_paralelo_mpi: syr2k_paralelo_mpi.c
	$(MPI) $(MPIFLAGS) -o syr2k_paralelo_mpi syr2k_paralelo_mpi.c

clean:
	rm -f syr2k syr2k_paralelo syr2k_paralelo_mpi
