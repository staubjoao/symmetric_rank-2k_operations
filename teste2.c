#include <stdio.h>
#include <mpi.h>

double alpha = 32412;
double beta = 2123;

double **A, **B, **C;

void alocarMatrizes(int ni, int nj)
{
    A = (double **)malloc(ni * sizeof(double *));
    for (int i = 0; i < nj; i++)
        A[i] = (double *)malloc(ni * sizeof(double));

    B = (double **)malloc(ni * sizeof(double *));
    for (int i = 0; i < nj; i++)
        B[i] = (double *)malloc(ni * sizeof(double));

    C = (double **)malloc(ni * sizeof(double *));
    for (int i = 0; i < ni; i++)
        C[i] = (double *)malloc(ni * sizeof(double));
}

void liberarMatrizes(int ni)
{
    for (int i = 0; i < ni; i++)
    {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }

    free(A);
    free(B);
    free(C);
}

double time_diff(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + 1e-6 * (end->tv_usec - start->tv_usec);
}

void init_array(int ni, int nj)
{
    int i, j;

    for (i = 0; i < ni; i++)
        for (j = 0; j < nj; j++)
        {
            A[i][j] = ((double)i * j) / ni;
            B[i][j] = ((double)i * j) / ni;
        }
    for (i = 0; i < ni; i++)
        for (j = 0; j < ni; j++)
            C[i][j] = ((double)i * j) / ni;
}

void print_array(int ni)
{
    int i, j;

    for (i = 0; i < ni; i++)
        for (j = 0; j < ni; j++)
        {
            fprintf(stderr, "%0.2lf ", C[i][j]);
            if ((i * ni + j) % 20 == 0)
                fprintf(stderr, "\n");
        }
    fprintf(stderr, "\n");
}

int main(int argc, char *argv[])
{
    int size;
    int rank;
    int i, j, k, from, to;
    int tamanho_matriz;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-d"))
        {
            if (strcmp(argv[i + 1], "small") == 0)
                // tamanho_matriz = 1600;
                tamanho_matriz = 8;
            else if (strcmp(argv[i + 1], "medium") == 0)
                // tamanho_matriz = 2400;
                tamanho_matriz = 4000;
            else if (strcmp(argv[i + 1], "large") == 0)
                // tamanho_matriz = 3200;
                tamanho_matriz = 4800;
            i++;
        }
    }

    if (tamanho_matriz % size != 0)
    {
        if (rank == 0)
            printf("Matrix size not divisible by number of processors\n");
        MPI_Finalize();
        exit(-1);
    }

    from = rank * tamanho_matriz / size;
    to = (rank + 1) * tamanho_matriz / size;

    if (rank == 0)
    {
        alocarMatrizes(tamanho_matriz, tamanho_matriz);
        init_array(tamanho_matriz, tamanho_matriz);
        printf("%d\n", tamanho_matriz);
    }

    MPI_Bcast(B, tamanho_matriz * tamanho_matriz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(A, tamanho_matriz * tamanho_matriz / size, MPI_DOUBLE, A[from], tamanho_matriz * tamanho_matriz / size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    printf("computing slice %d (from row %d to %d)\n", rank, from, to - 1);
    for (i = from; i < to; i++)
        for (j = 0; j < tamanho_matriz; j++)
        {
            C[i][j] = 0;
            for (k = 0; k < tamanho_matriz; k++)
                C[i][j] += A[i][k] * B[k][j];
        }

    MPI_Gather(C[from], tamanho_matriz * tamanho_matriz / size, MPI_DOUBLE, C, tamanho_matriz * tamanho_matriz / size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    printf("\nTeste\n");

    if (rank == 0)
    {
        printf("\n\n");
        for (i = 0; i < tamanho_matriz; i++)
        {
            printf("\n\t| ");
            for (j = 0; j < tamanho_matriz; j++)
                printf("%5d ", A[i][j]);
            printf("|");
        }
        printf("\n\n\t       * \n");
        for (i = 0; i < tamanho_matriz; i++)
        {
            printf("\n\t| ");
            for (j = 0; j < tamanho_matriz; j++)
                printf("%5d ", B[i][j]);
            printf("|");
        }
        printf("\n\n\t       = \n");
        for (i = 0; i < tamanho_matriz; i++)
        {
            printf("\n\t| ");
            for (j = 0; j < tamanho_matriz; j++)
                printf("%5d ", C[i][j]);
            printf("|");
        }
        printf("\n\n");

        liberarMatrizes(tamanho_matriz);
    }

    MPI_Finalize();
    return 0;
}