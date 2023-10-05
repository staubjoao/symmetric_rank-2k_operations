# Symmetric Rank-2k Operations Solver

Este projeto tem como objetivo desenvolver um programa que resolve o problema Symmetric Rank-2k Operations de forma paralela, utilizando as seguintes abordagens: pthreads, MPI e MPI + pthreads.

## Requisitos

Certifique-se de ter os seguintes requisitos instalados no seu sistema antes de compilar e executar o código:

- pthread library
- MPI (Message Passing Interface)

## Compilação

Para compilar o código, utilize o `makefile` fornecido no diretório raiz. Execute o seguinte comando no terminal, no diretório raiz do projeto:

```bash
make
```
Isso irá compilar todas as versões do programa, incluindo as implementações com pthreads, MPI e MPI + pthreads.

## Execução

Após a compilação bem-sucedida, você pode executar o programa escolhendo a abordagem desejada:

Para a versão MPI:

```bash
mpirun -np <número de processos> ./syr2k_mpi.o -d <tamanho>
```

Para a versão pthreads:

```bash
./syr2k_pthread.o -d <tamanho> -t <threads>
```

Para a versão MPI + pthreads:
```bash
mpirun -np <número de processos> ./syr2k_pthreads_mpi.o -d <tamanho> -t <threads>
```

## Testes 
Para executar testes automatizados, utilize o script test.sh localizado na pasta test. Este script permite que você execute testes para diferentes tamanhos de entrada e números de threads/processos. Para executar o script de teste, use o seguinte comando:
Para a versão MPI + pthreads:

```bash
./test.sh <tamanho (small, medium ou large)> <número de threads> <número de processos>
```

Certifique-se de substituir <tamanho>, <número de threads> e <número de processos> pelos valores desejados para o seu caso de teste.

Isso executara os testes além de comparar as saidas e apresentar o tempo do processamento e o tempo de toda execução de cada programa.

## Estrutura do Projeto

A estrutura do projeto está organizada da seguinte forma:

* src: Contém os arquivos-fonte do projeto.
* test: Contém os scripts e arquivos de teste.
* makefile: Arquivo de compilação.
* README.md: Este arquivo README.