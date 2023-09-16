#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <papi.h>

int tamanho_matriz;

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

void init_array(int ni, int nj,
                double *alpha,
                double *beta)
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

void kernel_syr2k(int ni, int nj,
                  double alpha,
                  double beta)
{
    int i, j, k;

    // Loop de código a ser medido
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

void instrucoes()
{
    printf("Use: \n");
    printf("-d TAMANHO (small, medium ou large)\n");
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Falta de parametros, use -h para verificar os comandos.\n");
        if (argc == 2 && strcmp(argv[1], "-h") == 0)
            instrucoes();
        return 1;
    }

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            if (strcmp(argv[i + 1], "small") == 0)
                tamanho_matriz = 320;
            else if (strcmp(argv[i + 1], "medium") == 0)
                tamanho_matriz = 400;
            else if (strcmp(argv[i + 1], "large") == 0)
                tamanho_matriz = 480;
            i++;
        }
    }

    int ni = tamanho_matriz;
    int nj = tamanho_matriz;

    double alpha;
    double beta;
    alocarMatrizes(ni, nj);

    init_array(ni, nj, &alpha, &beta);

    // Inicialize a biblioteca PAPI
    PAPI_library_init(PAPI_VER_CURRENT);

    // Crie um evento para medir o número de ciclos da CPU
    int event_set = PAPI_NULL;

    PAPI_create_eventset(&event_set);
    PAPI_add_event(event_set, PAPI_L1_DCM);

    // Inicie a medição
    PAPI_start(event_set);

    // Medir o tempo do kernel
    double start_time = PAPI_get_real_nsec();
    kernel_syr2k(ni, nj, alpha, beta);
    double end_time = PAPI_get_real_nsec();

    // Pare a medição
    PAPI_stop(event_set, NULL);

    // Leia o valor do contador
    long long values;
    PAPI_read(event_set, &values);

    // Imprima o número de ciclos da CPU
    double elapsed_time = (end_time - start_time) / 1e9; // Converter para segundos
    printf("Estagnação : %lld\n", values);
    printf("Elapsed time: %f\n", elapsed_time);

    // Libere recursos da PAPI
    PAPI_cleanup_eventset(event_set);
    PAPI_destroy_eventset(&event_set);
    PAPI_shutdown();

    // Libere a memória das matrizes
    liberarMatrizes(ni);

    return 0;
}

