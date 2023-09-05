#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 10 // Tamanho das matrizes A, B e C
double alpha = 32412;
double beta = 2123;

void allocateMatrix(double ***matrix, int rows, int cols)
{
    *matrix = (double **)malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++)
    {
        (*matrix)[i] = (double *)malloc(cols * sizeof(double));
    }
}

void syr2k(double **A, double **B, double **C, int inicio, int final)
{
    int i, j, k;
    for (i = inicio; i < final; i++)
        for (j = 0; j < N; j++)
            C[i][j] *= beta;
    MPI_Barrier(MPI_COMM_WORLD);
    for (i = inicio; i < final; i++)
        for (j = 0; j < N; j++)
            for (k = 0; k < N; k++)
            {
                C[i][j] += alpha * A[i][k] * B[j][k];
                C[i][j] += alpha * B[i][k] * A[j][k];
            }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    double **A = NULL;
    double **B = NULL;
    double **C = NULL;

    // Cada processo aloca sua parte das matrizes
    allocateMatrix(&A, N, N);
    allocateMatrix(&B, N, N);
    allocateMatrix(&C, N, N);

    // Inicialize as matrizes A e B com dados apropriados para cada processo
    // ...
    int i, j;
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
        {
            A[i][j] = ((double)i * j) / N;
            B[i][j] = ((double)i * j) / N;
        }
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            C[i][j] = ((double)i * j) / N;

    // Calcula a parte do SYR2K para cada processo
    int passo = N / num_procs;

    int inicio = rank * passo;
    int final = (rank == num_procs - 1) ? N : (rank + 1) * (N / num_procs);
    // printf("processo %d\t%d - %d\n", rank, inicio, final);
    syr2k(A, B, C, inicio, final);

    // Processo 0 recebe e combina as partes de C
    if (rank == 0)
    {
        for (int src = 1; src < num_procs; src++)
        {
            inicio = src * passo;
            final = (src == num_procs - 1) ? N : (src + 1) * passo;
            printf("%d: %d - %d\n", src, inicio, final);
            MPI_Recv(&(C[inicio][0]), (final - inicio) * N, MPI_DOUBLE, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    else
    {
        // Outros processos enviam suas partes de C para o processo 0
        MPI_Send(&(C[inicio][0]), (final - inicio) * N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    // Processo 0 imprime a matriz resultante C
    if (rank == 0)
    {
        int i, j;

        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
            {
                fprintf(stderr, "%0.2lf ", C[i][j]);
                if ((i * N + j) % 20 == 0)
                    fprintf(stderr, "\n");
            }
        fprintf(stderr, "\n");
    }

    // Libera a memória alocada
    for (int i = 0; i < N; i++)
    {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);

    MPI_Finalize();
    return 0;
}
