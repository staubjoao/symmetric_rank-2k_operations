CC = gcc
MPI = mpicc
CFLAGS = -O0 -w -g
MPIFLAGS = -g -Wall
PATH_SRC = ./src/
OBJS= syr2k.o syr2k_paralelo.o syr2k_paralelo_mpi.o syr2k_pthreads_mpi.o
all: $(OBJS)

syr2k.o: src/syr2k.c
	$(CC) $(CFLAGS) -lm -o syr2k.o $(PATH_SRC)syr2k.c

syr2k_paralelo.o: src/syr2k_paralelo.c
	$(CC) $(CFLAGS) -o syr2k_paralelo.o $(PATH_SRC)syr2k_paralelo.c

syr2k_paralelo_mpi.o: src/syr2k_paralelo_mpi.c
	$(MPI) $(MPIFLAGS) -o syr2k_paralelo_mpi.o $(PATH_SRC)syr2k_paralelo_mpi.c

syr2k_pthreads_mpi.o: src/syr2k_pthreads_mpi.c
	$(MPI) $(MPIFLAGS) -o syr2k_pthreads_mpi.o $(PATH_SRC)syr2k_pthreads_mpi.c

clear:
	rm -f *.o
