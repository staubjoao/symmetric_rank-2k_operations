#include <mpi.h>
#include <stdio.h>

#define SIZE 8 /* Size of matrices */

// double A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];
double **A, **B, **C;

void alocarMatrizes(int ni, int nj)
{
    // A = (double **)malloc(ni * sizeof(double *));
    // for (int i = 0; i < nj; i++)
    //     A[i] = (double *)malloc(ni * sizeof(double));

    // B = (double **)malloc(ni * sizeof(double *));
    // for (int i = 0; i < nj; i++)
    //     B[i] = (double *)malloc(ni * sizeof(double));

    // C = (double **)malloc(ni * sizeof(double *));
    // for (int i = 0; i < ni; i++)
    //     C[i] = (double *)malloc(ni * sizeof(double));

    A = (double **)malloc(ni * sizeof(double));
    A[0] = (double *)malloc(nj * ni * sizeof(double));
    for (size_t i = 0; i < nj; i++)
        A[i] = A[0] + nj * i;

    B = (double **)malloc(ni * sizeof(double));
    B[0] = (double *)malloc(nj * ni * sizeof(double));
    for (size_t i = 0; i < nj; i++)
        B[i] = B[0] + nj * i;

    C = (double **)malloc(ni * sizeof(double));
    C[0] = (double *)malloc(nj * ni * sizeof(double));
    for (size_t i = 0; i < nj; i++)
        C[i] = C[0] + nj * i;
}

// mpic++ teste.c -o teste.exe
// mpiexec -n 4 teste.exe

void fill_matrix(double **m)
{
    static int n = 0;
    int i, j;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            m[i][j] = (double)n++;
}

void print_matrix(double m[SIZE][SIZE])
{
    int i, j = 0;
    for (i = 0; i < SIZE; i++)
    {
        printf("\n\t| ");
        for (j = 0; j < SIZE; j++)
            printf("%2lf ", m[i][j]);
        printf("|");
    }
}

int main(int argc, char *argv[])
{
    int myrank, P, from, to, i, j, k;
    int tag = 666; /* any value will do */
    MPI_Status status;
    int tamanho_matriz;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank); /* who am i */
    MPI_Comm_size(MPI_COMM_WORLD, &P);      /* number of processors */

    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-d"))
        {
            if (strcmp(argv[i + 1], "small") == 0)
                // tamanho_matriz = 1600;
                tamanho_matriz = 3200;
            else if (strcmp(argv[i + 1], "medium") == 0)
                // tamanho_matriz = 2400;
                tamanho_matriz = 4000;
            else if (strcmp(argv[i + 1], "large") == 0)
                // tamanho_matriz = 3200;
                tamanho_matriz = 4800;
            i++;
        }
    }
    /* Just to use the simple variants of MPI_Gather and MPI_Scatter we */
    /* impose that SIZE is divisible by P. By using the vector versions, */
    /* (MPI_Gatherv and MPI_Scatterv) it is easy to drop this restriction. */

    if (SIZE % P != 0)
    {
        if (myrank == 0)
            printf("Matrix size not divisible by number of processors\n");
        MPI_Finalize();
        exit(-1);
    }

    from = myrank * SIZE / P;
    to = (myrank + 1) * SIZE / P;

    /* Process 0 fills the input matrices and broadcasts them to the rest */
    /* (actually, only the relevant stripe of A is sent to each process) */

    if (myrank == 0)
    {
        alocarMatrizes(SIZE, SIZE);
        fill_matrix(A);
        fill_matrix(B);
    }

    MPI_Bcast(B, SIZE * SIZE, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(A, SIZE * SIZE / P, MPI_DOUBLE, A[from], SIZE * SIZE / P, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    printf("computing slice %d (from row %d to %d)\n", myrank, from, to - 1);
    for (i = from; i < to; i++)
        for (j = 0; j < SIZE; j++)
        {
            C[i][j] = 0;
            for (k = 0; k < SIZE; k++)
                C[i][j] += A[i][k] * B[k][j];
        }

    MPI_Gather(C[from], SIZE * SIZE / P, MPI_DOUBLE, C, SIZE * SIZE / P, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (myrank == 0)
    {
        printf("\n\n");
        for (i = 0; i < SIZE; i++)
        {
            printf("\n\t| ");
            for (j = 0; j < SIZE; j++)
                printf("%2lf ", A[i][j]);
            printf("|");
        }
        printf("\n\n\t       * \n");
        for (i = 0; i < SIZE; i++)
        {
            printf("\n\t| ");
            for (j = 0; j < SIZE; j++)
                printf("%2lf ", B[i][j]);
            printf("|");
        }
        printf("\n\n\t       = \n");
        for (i = 0; i < SIZE; i++)
        {
            printf("\n\t| ");
            for (j = 0; j < SIZE; j++)
                printf("%2lf ", C[i][j]);
            printf("|");
        }
        printf("\n\n");
    }

    MPI_Finalize();
    return 0;
}