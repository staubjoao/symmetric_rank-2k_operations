#!/bin/bash

programa="../syr2k_pthread.o"
quantidade=$3

# Loop através dos tamanhos
for (( i=1; i<=quantidade; i++ )); do
    echo "Executando repetição $i para $1 e com $2 threads:"
    comando="perf stat -e cpu-clock,task-clock,context-switches,cpu-migrations,page-faults,branches,branch-misses,cache-references,cache-misses,cycles,instructions $programa -d $1 -t $2"             

    $comando
                
    echo "----------------------------------------"
done
