#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

// #include <polybench.h>

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

double time_diff(struct timeval *start, struct timeval *end)
{
  return (end->tv_sec - start->tv_sec) + 1e-6 * (end->tv_usec - start->tv_usec);
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

void kernel_syr2k(int ni, int nj,
                  double alpha,
                  double beta)
{
  int i, j, k;

#pragma scop
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
#pragma endscop
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
        tamanho_matriz = 10;
      else if (strcmp(argv[i + 1], "medium") == 0)
        tamanho_matriz = 4000;
      else if (strcmp(argv[i + 1], "large") == 0)
        tamanho_matriz = 4800;
      i++;
    }
  }

  int ni = tamanho_matriz;
  int nj = tamanho_matriz;

  double alpha;
  double beta;
  alocarMatrizes(ni, nj);

  init_array(ni, nj, &alpha, &beta);

  // polybench_start_instruments;

  struct timeval tstart, tend;

  gettimeofday(&tstart, NULL);
  kernel_syr2k(ni, nj,
               alpha, beta);
  gettimeofday(&tend, NULL);

  printf("Tempo sequencial: %lf sec\n", time_diff(&tstart, &tend));

  // polybench_stop_instruments;
  // polybench_print_instruments;

  print_array(ni);

  liberarMatrizes(ni);

  return 0;
}
