#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <sys/time.h>
#include <pthread.h>

// #include <polybench.h>

int tamanho_matriz, ni, nj;
int tamanho_matriz, n_threads, passo, resto;

pthread_barrier_t barreira;

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

void *kernel_syr2k_paralelo(void *arg)
{
    int id_thread = *((int *)arg);
    int inicio = id_thread * passo;
    int passolocal = passo;
    int i, j, k;

    if (id_thread == n_threads - 1)
        passolocal += resto;

#pragma scop
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
#pragma endscop
}

void instrucoes()
{
    printf("Use: \n");
    printf("-d TAMANHO (small, medium ou large)\n");
    printf("-t NUM_THREADS\n");
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        printf("Falta de parametros, use -h para verificar os comandos %d\n", argc);
        if (argc == 2 && strcmp(argv[1], "-h") == 0)
            instrucoes();
        return 1;
    }

    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-d"))
        {
            if (strcmp(argv[i + 1], "small") == 0)
                tamanho_matriz = 3200;
            else if (strcmp(argv[i + 1], "medium") == 0)
                tamanho_matriz = 4000;
            else if (strcmp(argv[i + 1], "large") == 0)
                tamanho_matriz = 4800;
            i++;
        }
        if (strcmp(argv[i], "-t") == 0)
            n_threads = atoi(argv[++i]);
    }

    ni = tamanho_matriz;
    nj = tamanho_matriz;

    alocarMatrizes(ni, nj);

    double alpha;
    double beta;

    init_array(ni, nj);

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

    // print_array(ni);

    liberarMatrizes(ni);

    return 0;
}
