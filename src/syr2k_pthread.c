#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <sys/time.h>
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
    {"threads", 't', "NUM", 0, "Specify number of threads"},
    {"debug", 'D', 0, 0, "Print debug information"},
    {"help", 'h', 0, 0, "Print this help message"},
    {0}};

static error_t parse_opt(
    int key,   /* entrada */
    char *arg, /* entrada */
    struct argp_state *state /* saida */)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
    case 'd':
        arguments->size = arg;
        break;
    case 't':
        arguments->num_threads = atoi(arg);
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

void alocar_matrizes(int ni, int nj);
void liberar_matrizes(int ni);
double time_diff(struct timeval *start, struct timeval *end);
void inicia_matrizes(int ni, int nj);
void imprimir_matriz_resultante(int ni);
void *kernel_syr2k_paralelo(void *arg);

int tamanho_matriz, ni, nj;
int tamanho_matriz, n_threads, passo, resto;

pthread_barrier_t barreira;

double alpha = 32412;
double beta = 2123;

double **A, **B, **C;

int main(int argc, char **argv)
{
    struct arguments arguments;
    arguments.size = NULL;
    arguments.num_threads = 0;
    arguments.debug = 0;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.size == NULL || arguments.num_threads == 0)
    {
        fprintf(stderr, "Os argumentos -d e -t são obrigatórios. Use -h para ver os comandos.\n");
        exit(1);
    }

    if (arguments.size != NULL)
    {
        if (strcmp(arguments.size, "small") == 0)
            tamanho_matriz = 1024;
        else if (strcmp(arguments.size, "medium") == 0)
            tamanho_matriz = 2048;
        else if (strcmp(arguments.size, "large") == 0)
            tamanho_matriz = 4096;
    }
    if (arguments.num_threads > 0)
    {
        n_threads = arguments.num_threads;
    }

    ni = tamanho_matriz;
    nj = tamanho_matriz;

    alocar_matrizes(ni, nj);

    double alpha;
    double beta;

    inicia_matrizes(ni, nj);

    struct timeval tstart, tend;

    passo = (ni / n_threads);
    resto = (ni % n_threads);
    pthread_t threads[n_threads];
    int id_thread[n_threads];

    pthread_barrier_init(&barreira, NULL, n_threads);

    gettimeofday(&tstart, NULL);
    for (int t = 0; t < n_threads; ++t)
    {
        id_thread[t] = t;
        pthread_create(&threads[t], NULL, kernel_syr2k_paralelo, (void *)&id_thread[t]);
    }

    for (int t = 0; t < n_threads; ++t)
        pthread_join(threads[t], NULL);
    gettimeofday(&tend, NULL);

    printf("Tempo paralelo: %lf sec\n", time_diff(&tstart, &tend));

    if (arguments.debug)
        imprimir_matriz_resultante(ni);

    liberar_matrizes(ni);

    return 0;
}

void alocar_matrizes(
    int ni /* entrada */,
    int nj /* entrada */)
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

void liberar_matrizes(
    int ni /* entrada */)
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

double time_diff(
    struct timeval *start /* entrada */,
    struct timeval *end /* entrada */)
{
    return (end->tv_sec - start->tv_sec) + 1e-6 * (end->tv_usec - start->tv_usec);
}

void inicia_matrizes(
    int ni /* entrada */,
    int nj /* entrada */)
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

void imprimir_matriz_resultante(
    int ni /* entrada */)
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

void *kernel_syr2k_paralelo(
    void *arg /* entrada */)
{
    int id_thread = *((int *)arg);
    int inicio = id_thread * passo;
    int passolocal = passo;
    int i, j, k;

    if (id_thread == n_threads - 1)
        passolocal += resto;

    for (i = inicio; i < inicio + passolocal; i++)
        for (j = 0; j < ni; j++)
            C[i][j] *= beta;
    pthread_barrier_wait(&barreira);
    for (i = inicio; i < inicio + passolocal; i++)
        for (j = 0; j < ni; j++)
            for (k = 0; k < nj; k++)
            {
                C[i][j] += alpha * A[i][k] * B[j][k];
                C[i][j] += alpha * B[i][k] * A[j][k];
            }
}
