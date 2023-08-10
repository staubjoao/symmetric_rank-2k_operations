#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <polybench.h>

#include "syr2k.h"
#include <stdlib.h>

int tamanho_matriz;

static void init_array(int ni, int nj,
                       double *alpha,
                       double *beta,
                       double POLYBENCH_2D(C, tamanho_matriz, tamanho_matriz, ni, ni),
                       double POLYBENCH_2D(A, tamanho_matriz, tamanho_matriz, ni, nj),
                       double POLYBENCH_2D(B, tamanho_matriz, tamanho_matriz, ni, nj))
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

static void print_array(int ni,
                        double POLYBENCH_2D(C, tamanho_matriz, tamanho_matriz, ni, ni))
{
  int i, j;

  for (i = 0; i < ni; i++)
    for (j = 0; j < ni; j++)
    {
      fprintf(stderr, DATA_PRINTF_MODIFIER, C[i][j]);
      if ((i * ni + j) % 20 == 0)
        fprintf(stderr, "\n");
    }
  fprintf(stderr, "\n");
}

static void kernel_syr2k(int ni, int nj,
                         double alpha,
                         double beta,
                         double POLYBENCH_2D(C, tamanho_matriz, tamanho_matriz, ni, ni),
                         double POLYBENCH_2D(A, tamanho_matriz, tamanho_matriz, ni, nj),
                         double POLYBENCH_2D(B, tamanho_matriz, tamanho_matriz, ni, nj))
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
  // printf("-t NUM_THREADS\n");
}

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    printf("Falta de parametros, use -h para verificar os comandos %d\n", argc);
    if (argc == 2 && strcmp(argv[1], "-h") == 0)
      instrucoes();
    return 1;
  }

  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "-d") == 0)
    {
      if (strcmp(argv[i + 1], "small") == 0)
        tamanho_matriz = 32;
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
  POLYBENCH_2D_ARRAY_DECL(C, double, tamanho_matriz, tamanho_matriz, ni, ni);
  POLYBENCH_2D_ARRAY_DECL(A, double, tamanho_matriz, tamanho_matriz, ni, nj);
  POLYBENCH_2D_ARRAY_DECL(B, double, tamanho_matriz, tamanho_matriz, ni, nj);

  init_array(ni, nj, &alpha, &beta,
             POLYBENCH_ARRAY(C),
             POLYBENCH_ARRAY(A),
             POLYBENCH_ARRAY(B));

  polybench_start_instruments;

  kernel_syr2k(ni, nj,
               alpha, beta,
               POLYBENCH_ARRAY(C),
               POLYBENCH_ARRAY(A),
               POLYBENCH_ARRAY(B));

  polybench_stop_instruments;
  polybench_print_instruments;

  polybench_prevent_dce(print_array(ni, POLYBENCH_ARRAY(C)));

  POLYBENCH_FREE_ARRAY(C);
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(B);

  return 0;
}
