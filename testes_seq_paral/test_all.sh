#!/bin/bash

tamanhos=("small" "medium" "large")
num_threads=("2" "4" "8" "16")
quantidade=5

path_sequencial="logs_sequencial"
path_paralelo="logs_paralelo"
path_mpi="logs_mpi"
path_all="test_staub_new_n"

mkdir -p $path_all

echo "Executando o sequencial"

path_total_sequencial="${path_all}/${path_sequencial}"

mkdir -p $path_total_sequencial

path_total_paralelo="${path_all}/${path_paralelo}"

mkdir -p $path_total_paralelo

path_total_mpi="${path_all}/${path_mpi}"

mkdir -p $path_total_mpi

for tamanho in "${tamanhos[@]}"; do
    arquivo="${path_all}/${path_sequencial}/log_seq_${tamanho}_out.txt"
    echo "$arquivo"
    ./test_seq.sh $tamanho $quantidade > $arquivo 2>&1   
done

echo "Finnal da execução sequencial"

echo "Executando o paralelo"


for tamanho in "${tamanhos[@]}"; do
    for threads in "${num_threads[@]}"; do
        arquivo="${path_all}/${path_paralelo}/log_paralelo_${tamanho}_${threads}_out.txt"
        echo "$arquivo"
        ./test_pthread.sh $tamanho $threads $quantidade > $arquivo 2>&1
    done
done

echo "Finnal da execução paralela"


echo "Finnal da execução sequencial"

echo "Executando o MPI"

for tamanho in "${tamanhos[@]}"; do
    for threads in "${num_threads[@]}"; do
        arquivo="${path_all}/${path_mpi}/log_paralelo_${tamanho}_${threads}_out.txt"
        echo "$arquivo"
        ./test_mpi.sh $tamanho $threads $quantidade > $arquivo 2>&1
    done
done

echo "Finnal da execução MPI"
