#!/bin/bash

programa="../syr2k"

for (( i=1; i<=$3; i++ )); do
    echo "Executando repetição $i para $1:"
    comando="perf stat -e cpu-clock,task-clock,context-switches,cpu-migrations,page-faults,branches,branch-misses,cache-references,cache-misses,cycles,instructions $programa -d $1"             
            
    $comando
    echo "----------------------------------------"
done
