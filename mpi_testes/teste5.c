/*
MPI parallel matrix multiplication using asynchronous message passing.
Each process will be assigned a number of rows.

**** rank 0 process responsible for first interval of
**** the matrix as well as the remainder.

The message passing calls used include synchronous as well asynchronous
send and receive, plus broadcast.

For simplicity, square matrices will be assumed and static, global arrays
used.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// macro prints array AA, size rzXcz, using formated string fs.
// use as in PRINT2M(A,4,5,"%f ")
#define PRINT2M(AA, rz, cz, fs)       \
    {                                 \
        int i, j;                     \
        for (i = 0; i < rz; i++)      \
        {                             \
            for (j = 0; j < cz; j++)  \
                printf(fs, AA[i][j]); \
            printf("\n");             \
        }                             \
    }

// size of NxN matrix:
#define N 500

// global static structures
double A[N][N];
double B[N][N];
double AB[N][N]; /* to hold result of A*B */

// conventional one-processor solution
void cmul()
{
    int i, j, k;
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            for (k = 0, AB[i][j] = 0; k < N; k++)
                AB[i][j] += A[i][k] * B[k][j];
} // cmul

void compinterval(int, int); // prototype

int main(int argc, char **argv)
{
    int rank, size, interval, remainder, i, j;
    double time1, time2, time3; // for timing measurements
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Request ireq[128]; // , asynch request, assume size<128
    MPI_Status stat;       // status of asynch communication

    // compute interval size each process is responsible for,
    // rank 0 process will be responsible for the remainder
    interval = N / (size);
    remainder = N % (size);

    if (rank == 0) // root/coordinator process
    {
        if (argc > 1)
            srandom(atoi(*(argv + 1))); // random seed
        // generate random matrices
        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
            {
                A[i][j] = random() % 4;
                B[i][j] = random() % 4;
            } // for

        time1 = MPI_Wtime(); // record time
        // For array B, we broadcast the whole array, however, the Bcast
        // operation is strange because it needs to be executed by all
        // processes.  There is no corresponding Recv for a Bcast
        MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD); // send broadcast
        //      printf("%d: Bcast complete\n",rank);

        printf("Teste\n");
        // Send intervals of array A to worker processes
        for (i = 1; i < size; i++)
            MPI_Isend(A + (i * interval), interval * N, MPI_DOUBLE, i, i,
                      MPI_COMM_WORLD, ireq + i);
        for (i = 1; i < size; i++)
            MPI_Waitany(size, ireq, &j, &stat); // join on sends

        compinterval(0, interval);                // local work
        compinterval(size * interval, remainder); // remainder

        // get results from workers:
        for (i = 1; i < size; i++)
            MPI_Irecv(AB + (i * interval), interval * N, MPI_DOUBLE, i, i,
                      MPI_COMM_WORLD, ireq + i);
        for (i = 1; i < size; i++)
        {
            MPI_Waitany(size, ireq, &j, &stat);
            //	printf("received results from process %d\n",j);
        }

        time2 = MPI_Wtime(); // record time

        // run conventional algorithm:
        cmul();
        time3 = MPI_Wtime();

        printf("approx %d-process time Tp: %f sec.\n", size, time2 - time1);
        printf("approx 1-process (conventional) time T1: %f sec.\n", time3 - time2);
        printf("efficiency : %f\n", (time3 - time2) / ((time2 - time1) * size));
        /* print :
        PRINT2M(A,N,N,"%.0f ");  printf("------\n");
        PRINT2M(B,N,N,"%.0f ");  printf("------\n");
        PRINT2M(AB,N,N,"%.0f ");
        */
    }
    else // worker process
    {
        MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD); // receive broadcast
        // synchronous receive
        MPI_Recv(A + (rank * interval), interval * N, MPI_DOUBLE, 0, rank,
                 MPI_COMM_WORLD, &stat);
        compinterval(rank * interval, interval);
        // send results back to root process, synchronous send
        MPI_Send(AB + (rank * interval), interval * N, MPI_DOUBLE, 0, rank,
                 MPI_COMM_WORLD);
    }
    MPI_Finalize();
} // main

// multiply fragment of matrix
void compinterval(int start, int interval)
{
    int i, j, k;
    for (i = start; i < start + interval; i++)
        for (j = 0; j < N; j++)
        {
            AB[i][j] = 0;
            for (k = 0; k < N; k++)
                AB[i][j] += A[i][k] * B[k][j];
        }
}