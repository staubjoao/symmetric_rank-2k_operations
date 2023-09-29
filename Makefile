CC = gcc
MPI = mpicc
MPIFLAGS = -g -Wall
PATH_SRC = ./src/
OBJS= syr2k_seq.o syr2k_pthread.o syr2k_mpi.o syr2k_pthreads_mpi.o
all: $(OBJS)

syr2k_seq.o: src/syr2k_seq.c
	$(CC) -o syr2k_seq.o $(PATH_SRC)syr2k_seq.c

syr2k_pthread.o: src/syr2k_pthread.c
	$(CC) -o syr2k_pthread.o $(PATH_SRC)syr2k_pthread.c

syr2k_mpi.o: src/syr2k_mpi.c
	$(MPI) $(MPIFLAGS) -o syr2k_mpi.o $(PATH_SRC)syr2k_mpi.c

syr2k_pthreads_mpi.o: src/syr2k_pthreads_mpi.c
	$(MPI) $(MPIFLAGS) -o syr2k_pthreads_mpi.o $(PATH_SRC)syr2k_pthreads_mpi.c

clear:
	rm -f *.o
