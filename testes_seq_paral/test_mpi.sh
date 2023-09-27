#!/bin/bash

repeticoes=1

programa="../syr2k_paralelo_mpi"

# Loop através dos tamanhos
for (( i=1; i<=repeticoes; i++ )); do
    echo "Executando repetição $i para $1 e com $2 threads:"
    comando="perf stat -e cpu-clock,task-clock,context-switches,cpu-migrations,page-faults,branches,branch-misses,cache-references,cache-misses,cycles,instructions mpirun -np $2 $programa -d $1"             

    $comando
                
    echo "----------------------------------------"
done
