#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include <pthread.h>

#include <argp.h>

struct arguments
{
    char *size;
    int num_threads;
    int debug;
};

static struct argp_option options[] = {
    {"size", 'd', "SIZE", 0, "Specify matrix size (small, medium or large)"},
    {"debug", 'D', 0, 0, "Print debug information"},
    {"help", 'h', 0, 0, "Print this help message"},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
    case 'd':
        arguments->size = arg;
        break;
    case 'D':
        arguments->debug = 1;
        break;
    case 'h':
        argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
        exit(0);
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, NULL, NULL};

void verificar_erro(int teste_local, char fname[], char mensagem[], MPI_Comm comm);
void alocar_matrizes(double **A_local, double **B_local, double **C_local, int n, int n_local, MPI_Comm comm);
void imprimir_matriz_debug(char titulo[], double matriz_local[], int n, int n_local, int rank, MPI_Comm comm);
void inicia_matrizes(double A_local[], double B_local[], double C_local[], int n, int n_local, int rank, MPI_Comm comm);
void imprimir_matriz_resultante(double C_local[], int n, int n_local, int rank, double start, double end, int debug, MPI_Comm comm);
void kernel_syr2k_mpi_aux(int n_local, MPI_Comm comm);

const double alpha = 32412;
const double beta = 2123;

double *A, *B, *C;
double *A_local, *B_local, *C_local;
int n_threads, passo, resto;
int n;

pthread_barrier_t barreira;

void *kernel_syr2k_trabalhador(void *arg)
{
    int id_thread = *((int *)arg);
    int inicio = id_thread * passo;
    int passolocal = passo;
    int i, j, k;

    if (id_thread == n_threads - 1)
        passolocal += resto;

    for (i = inicio; i < inicio + passolocal; i++)
        for (j = 0; j < n; j++)
            C_local[i * n + j] *= beta;
    pthread_barrier_wait(&barreira);
    for (i = inicio; i < inicio + passolocal; i++)
    {
        for (j = 0; j < n; j++)
        {
            for (k = 0; k < n; k++)
            {
                C_local[i * n + j] += alpha * A_local[i * n + k] * B[j * n + k];
                C_local[i * n + j] += alpha * B_local[i * n + k] * A[j * n + k];
            }
        }
    }
}

int main(int argc, char **argv)
{
    int n_local, debug;
    int rank, size, root = 0;
    MPI_Comm comm;

    MPI_Init(NULL, NULL);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    if (rank == root)
    {
        struct arguments arguments;
        arguments.size = NULL;
        arguments.num_threads = 0;
        arguments.debug = 0;

        argp_parse(&argp, argc, argv, 0, 0, &arguments);

        if (arguments.size == NULL)
        {
            fprintf(stderr, "O argumento -d. Use -h para ver os comandos.\n");
            exit(1);
        }

        if (arguments.size != NULL)
        {
            if (strcmp(arguments.size, "small") == 0)
                // n = 3200;
                n = 8;
            else if (strcmp(arguments.size, "medium") == 0)
                // n = 4432;
                n = 16;
            else if (strcmp(arguments.size, "large") == 0)
                // n = 5664;
                n = 32;
        }
        debug = arguments.debug;

        n_threads = 2;
    }

    MPI_Bcast(&n, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Bcast(&debug, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Bcast(&n_threads, 1, MPI_INT, root, MPI_COMM_WORLD);

    n_local = n / size;

    alocar_matrizes(&A_local, &B_local, &C_local, n, n_local, comm);
    inicia_matrizes(A_local, B_local, C_local, n, n_local, rank, comm);

    double start = MPI_Wtime();
    kernel_syr2k_mpi_aux(n_local, comm);
    double end = MPI_Wtime();

    imprimir_matriz_resultante(C_local, n, n_local, rank, start, end, debug, comm);

    free(A_local);
    free(B_local);
    free(C_local);
    MPI_Finalize();
    return 0;
}

void verificar_erro(
    int teste_local /* entrada */,
    char fname[] /* entrada */,
    char mensagem[] /* entrada */,
    MPI_Comm comm /* entrada */)
{
    int ok;

    MPI_Allreduce(&teste_local, &ok, 1, MPI_INT, MPI_MIN, comm);
    if (ok == 0)
    {
        int rank;
        MPI_Comm_rank(comm, &rank);
        if (rank == 0)
        {
            fprintf(stderr, "Processo %d > em %s, %s\n", rank, fname, mensagem);
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
    char titulo[] /* entrada */,
    double matriz_local[] /* entrada */,
    int n /* entrada */,
    int n_local /* entrada */,
    int rank /* entrada */,
    MPI_Comm comm /* entrada */)
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
    double C_local[] /* entrada */,
    int n /* entrada */,
    int n_local /* entrada */,
    int rank /* entrada */,
    double start,
    double end,
    int debug,
    MPI_Comm comm /* entrada */)
{
    double *C = NULL;
    int i, j, teste_local = 1;

    if (rank == 0)
    {
        C = malloc(n * n * sizeof(double));
        if (C == NULL)
            teste_local = 0;
        verificar_erro(teste_local, "imprimir_matriz_resultante", "Não foi possivel alocar a matriz localmente", comm);
        MPI_Gather(C_local, n_local * n, MPI_DOUBLE, C, n_local * n, MPI_DOUBLE, 0, comm);

        printf("Tempo mpi: %lf sec\n", end - start);

        if (debug)
        {

            for (i = 0; i < n; i++)
                for (j = 0; j < n; j++)
                {
                    fprintf(stderr, "%0.2lf ", C[i * n + j]);
                    if ((i * n + j) % 20 == 0)
                        fprintf(stderr, "\n");
                }
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "\nteste\n");
        free(C);
    }
    else
    {
        verificar_erro(teste_local, "imprimir_matriz_resultante", "Não foi possivel alocar a matriz localmente", comm);
        MPI_Gather(C_local, n_local * n, MPI_DOUBLE, C, n_local * n, MPI_DOUBLE, 0, comm);
    }
}

void kernel_syr2k_mpi_aux(
    int n_local /* entrada  */,
    MPI_Comm comm /* entrada  */)
{
    int teste_local = 1;

    A = malloc(n * n * sizeof(double));
    B = malloc(n * n * sizeof(double));
    if (A == NULL || B == NULL)
        teste_local = 0;
    verificar_erro(teste_local, "kernel_syr2k", "Não foi possivel alocar as matrizes localmente", comm);

    MPI_Allgather(A_local, n_local * n, MPI_DOUBLE, A, n_local * n, MPI_DOUBLE, comm);
    MPI_Allgather(B_local, n_local * n, MPI_DOUBLE, B, n_local * n, MPI_DOUBLE, comm);

    passo = (n / n_threads);
    resto = (n % n_threads);
    pthread_t threads[n_threads];
    int id_thread[n_threads];

    pthread_barrier_init(&barreira, NULL, n_threads);

    for (int t = 0; t < n_threads; ++t)
    {
        id_thread[t] = t;
        pthread_create(&threads[t], NULL, kernel_syr2k_trabalhador, (void *)&id_thread[t]);
    }

    for (int t = 0; t < n_threads; ++t)
        pthread_join(threads[t], NULL);

    free(A);
    free(B);
}