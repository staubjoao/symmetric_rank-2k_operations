#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <argp.h>

struct arguments
{
    char *size;
    int debug;
};

static struct argp_option options[] = {
    {"size", 'd', "SIZE", 0, "Specify matrix size (small, medium, or large)"},
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
void init_array(int ni, int nj, double *alpha, double *beta);
void kernel_syr2k(int ni, int nj, double alpha, double beta);
void imprimir_matriz_resultante(int ni);

int tamanho_matriz;

double **A, **B, **C;

int main(int argc, char **argv)
{
    struct arguments arguments;
    arguments.size = NULL;
    arguments.debug = 0;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.size == NULL)
    {
        fprintf(stderr, "O argumento -d é obrigatório. Use -h para ver os comandos.\n");
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

    int ni = tamanho_matriz;
    int nj = tamanho_matriz;

    double alpha;
    double beta;
    alocar_matrizes(ni, nj);

    init_array(ni, nj, &alpha, &beta);

    struct timeval tstart, tend;

    gettimeofday(&tstart, NULL);
    kernel_syr2k(ni, nj, alpha, beta);
    gettimeofday(&tend, NULL);

    printf("Tempo sequencial: %lf sec %d\n", time_diff(&tstart, &tend), arguments.debug);

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
    int ni /* entrada */
)
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

void init_array(
    int ni /* entrada */, int nj, double *alpha, double *beta)
{
    int i, j;

    *alpha = 32412;
    *beta = 2123;
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

void kernel_syr2k(
    int ni /* entrada */,
    int nj /* entrada */,
    double alpha /* entrada */,
    double beta /* entrada */)
{
    int i, j, k;

    for (i = 0; i < ni; i++)
        for (j = 0; j < ni; j++)
            C[i][j] *= beta;
    for (i = 0; i < ni; i++)
        for (j = 0; j < ni; j++)
            for (k = 0; k < nj; k++)
            {
                C[i][j] += alpha * A[i][k] * B[j][k];
                C[i][j] += alpha * B[i][k] * A[j][k];
            }
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
