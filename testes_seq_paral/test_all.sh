#!/bin/bash

tamanhos=("small" "medium" "large")
num_threads=("2" "4" "8" "16")
path_sequencial="logs_sequencial"
path_paralelo="logs_paralelo"
path_all="test_staub"

rm -rf $path_all

mkdir $path_all


echo "Executando o sequencial"

rm -rf $path_sequencial

mkdir $path_sequencial

for tamanho in "${tamanhos[@]}"; do
    arquivo="${path_all}/${path_sequencial}/log_seq_${tamanho}_out.txt"
    echo "$arquivo"
    ./test_sy2k_seq.sh $tamanho > $arquivo 2>&1   
done

echo "Finnal da execução sequencial"

echo "Executando o paralelo"

rm -rf $path_paralelo

mkdir $path_paralelo

for tamanho in "${tamanhos[@]}"; do
    for threads in "${num_threads[@]}"; do
        arquivo="${path_all}/${path_paralelo}/log_paralelo_${tamanho}_${threads}_out.txt"
        echo "$arquivo"
        ./test_sy2k_paralelo.sh $tamanho $threads > $arquivo 2>&1
    done
done

echo "Finnal da execução paralela"


