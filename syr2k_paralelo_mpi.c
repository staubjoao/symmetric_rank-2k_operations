#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void verificar_erro(int teste_local, char fname[], char mensagem[], MPI_Comm comm);
void alocar_matrizes(double **A_local, double **B_local, double **C_local, int n, int n_local, MPI_Comm comm);
void imprimir_matriz_debug(char titulo[], double matriz_local[], int n, int n_local, int rank, MPI_Comm comm);
void inicia_matrizes(double A_local[], double B_local[], double C_local[], int n, int n_local, int rank, MPI_Comm comm);
void imprimir_matriz_resultante(double C_local[], int n, int n_local, int rank, MPI_Comm comm);
void kernel_syr2k(double A_local[], double B_local[], double C_local[], int n, int n_local, int rank, MPI_Comm comm);

const double alpha = 32412;
const double beta = 2123;

int main(void)
{
    double *A_local;
    double *B_local;
    double *C_local;
    int n, n_local;
    int rank, size;
    MPI_Comm comm;

    MPI_Init(NULL, NULL);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    n = 32;

    n_local = n / size;

    alocar_matrizes(&A_local, &B_local, &C_local, n, n_local, comm);
    inicia_matrizes(A_local, B_local, C_local, n, n_local, rank, comm);

    kernel_syr2k(A_local, B_local, C_local, n, n_local, rank, comm);

    imprimir_matriz_resultante(C_local, n, n_local, rank, comm);

    free(A_local);
    free(B_local);
    free(C_local);
    MPI_Finalize();
    return 0;
}

void verificar_erro(
    int teste_local /* in */,
    char fname[] /* in */,
    char mensagem[] /* in */,
    MPI_Comm comm /* in */)
{
    int ok;

    MPI_Allreduce(&teste_local, &ok, 1, MPI_INT, MPI_MIN, comm);
    if (ok == 0)
    {
        int rank;
        MPI_Comm_rank(comm, &rank);
        if (rank == 0)
        {
            fprintf(stderr, "Processo %d > em %s, %s\n", rank, fname,
                    mensagem);
            fflush(stderr);
        }
        MPI_Finalize();
        exit(-1);
    }
}

void alocar_matrizes(
    double **A_local /* saida */,
    double **B_local /* saida */,
    double **C_local /* saida */,
    int n /* entrada  */,
    int n_local /* entrada  */,
    MPI_Comm comm /* entrada  */)
{

    int teste_local = 1;

    *A_local = malloc(n_local * n * sizeof(double));
    *B_local = malloc(n_local * n * sizeof(double));
    *C_local = malloc(n_local * n * sizeof(double));

    if (*A_local == NULL || B_local == NULL ||
        C_local == NULL)
        teste_local = 0;
    verificar_erro(teste_local, "alocar_matrizes", "Não foi possivel alocar as matrizes", comm);
}

void imprimir_matriz_debug(
    char titulo[] /* in */,
    double matriz_local[] /* in */,
    int n /* in */,
    int n_local /* in */,
    int rank /* in */,
    MPI_Comm comm /* in */)
{
    double *matriz = NULL;
    int i, j, teste_local = 1;

    if (rank == 0)
    {
        matriz = malloc(n * n * sizeof(double));
        if (matriz == NULL)
            teste_local = 0;
        verificar_erro(teste_local, "imprimir_matriz_resultante", "Não foi possivel alocar a matriz localmente", comm);
        MPI_Gather(matriz_local, n_local * n, MPI_DOUBLE,
                   matriz, n_local * n, MPI_DOUBLE, 0, comm);
        printf("\nThe matrix %s\n", titulo);
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
                printf("%0.2lf ", matriz[i * n + j]);
            printf("\n");
        }
        printf("\n");
        free(matriz);
    }
    else
    {
        verificar_erro(teste_local, "imprimir_matriz_resultante", "Não foi possivel alocar a matriz localmente", comm);
        MPI_Gather(matriz_local, n_local * n, MPI_DOUBLE,
                   matriz, n_local * n, MPI_DOUBLE, 0, comm);
    }
}

void inicia_matrizes(
    double A_local[] /* saida */,
    double B_local[] /* saida */,
    double C_local[] /* saida */,
    int n /* entrada  */,
    int n_local /* entrada  */,
    int rank /* entrada  */,
    MPI_Comm comm /* entrada  */)
{
    double *A = NULL;
    double *B = NULL;
    double *C = NULL;
    int teste_local = 1;
    int i, j;

    if (rank == 0)
    {
        A = malloc(n * n * sizeof(double));
        B = malloc(n * n * sizeof(double));
        C = malloc(n * n * sizeof(double));
        if (A == NULL)
            teste_local = 0;
        verificar_erro(teste_local, "inicia_matrizes", "Não foi possivel alocar as matrizes localmente", comm);
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
            {
                A[i * n + j] = ((double)i * j) / n;
                B[i * n + j] = ((double)i * j) / n;
                C[i * n + j] = ((double)i * j) / n;
            }
        MPI_Scatter(A, n_local * n, MPI_DOUBLE, A_local, n_local * n, MPI_DOUBLE, 0, comm);
        MPI_Scatter(B, n_local * n, MPI_DOUBLE, B_local, n_local * n, MPI_DOUBLE, 0, comm);
        MPI_Scatter(C, n_local * n, MPI_DOUBLE, C_local, n_local * n, MPI_DOUBLE, 0, comm);
        free(A);
        free(B);
        free(C);
    }
    else
    {
        verificar_erro(teste_local, "inicia_matrizes", "Não foi possivel alocar as matrizes localmente", comm);
        MPI_Scatter(A, n_local * n, MPI_DOUBLE, A_local, n_local * n, MPI_DOUBLE, 0, comm);
        MPI_Scatter(B, n_local * n, MPI_DOUBLE, B_local, n_local * n, MPI_DOUBLE, 0, comm);
        MPI_Scatter(C, n_local * n, MPI_DOUBLE, C_local, n_local * n, MPI_DOUBLE, 0, comm);
    }
}

void imprimir_matriz_resultante(
    double C_local[] /* in */,
    int n /* in */,
    int n_local /* in */,
    int rank /* in */,
    MPI_Comm comm /* in */)
{
    double *C = NULL;
    int i, j, teste_local = 1;

    if (rank == 0)
    {
        C = malloc(n * n * sizeof(double));
        if (C == NULL)
            teste_local = 0;
        verificar_erro(teste_local, "imprimir_matriz_resultante", "Não foi possivel alocar a matriz localmente", comm);
        MPI_Gather(C_local, n_local * n, MPI_DOUBLE,
                   C, n_local * n, MPI_DOUBLE, 0, comm);

        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
                fprintf(stderr, "%0.2lf ", C[i * n + j]);
            fprintf(stderr, "\n");
        }
        free(C);
    }
    else
    {
        verificar_erro(teste_local, "imprimir_matriz_resultante", "Não foi possivel alocar a matriz localmente", comm);
        MPI_Gather(C_local, n_local * n, MPI_DOUBLE,
                   C, n_local * n, MPI_DOUBLE, 0, comm);
    }
}

void kernel_syr2k(
    double A_local[] /* entrada  */,
    double B_local[] /* entrada  */,
    double C_local[] /* in and saida  */,
    int n /* entrada  */,
    int n_local /* entrada  */,
    int rank /* in */,
    MPI_Comm comm /* entrada  */)
{
    double *A, *B;
    int i, j, k;
    int teste_local = 1;

    A = malloc(n * n * sizeof(double));
    B = malloc(n * n * sizeof(double));
    if (A == NULL || B == NULL)
        teste_local = 0;
    verificar_erro(teste_local, "kernel_syr2k", "Não foi possivel alocar as matrizes localmente", comm);

    MPI_Allgather(A_local, n_local * n, MPI_DOUBLE, A, n_local * n, MPI_DOUBLE, comm);
    MPI_Allgather(B_local, n_local * n, MPI_DOUBLE, B, n_local * n, MPI_DOUBLE, comm);

    for (i = 0; i < n_local; i++)
        for (j = 0; j < n; j++)
            C_local[i * n + j] *= beta;
    MPI_Barrier(comm);

    for (i = 0; i < n_local; i++)
        for (j = 0; j < n; j++)
            for (k = 0; k < n; k++)
            {
                C_local[i * n + j] += alpha * A_local[i * n + k] * B[j * n + k];
                C_local[i * n + j] += alpha * B_local[i * n + k] * A[j * n + k];
            }

    free(A);
    free(B);
}