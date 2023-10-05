#!/bin/bash

tamanho=$1
num_threads=$2
num_processos=$3

programa_sequencial="../syr2k_seq.o"
programa_paralelo="../syr2k_pthread.o"
programa_mpi="../syr2k_mpi.o"
programa_pthread_mpi="../syr2k_pthreads_mpi.o"

testes_log="teste_log"

mkdir -p $testes_log

arquivo1="saida_seq_${tamanho}.out"
if [ ! -e ${testes_log}/${arquivo1} ]; then
    echo "O arquivo ${arquivo1} não existe. Executando o código sequencial com o tamanho ${tamanho}"
    comando="time $programa_sequencial -d ${tamanho} -D 2> ${testes_log}/${arquivo1}"

    eval $comando
fi

arquivo2="saida_pthreads_${tamanho}_${num_threads}.out"
if [ ! -e ${testes_log}/${arquivo2} ]; then
    echo "O arquivo ${arquivo2} não existe. Executando o código utilizando o pthreads com o tamanho ${tamanho} e ${num_threads} threads"
    comando="time $programa_paralelo -d ${tamanho} -t ${num_threads} -D 2> ${testes_log}/${arquivo2}"

    echo $comando

    eval $comando
fi

arquivo3="saida_mpi_${tamanho}_${num_processos}.out"
if [ ! -e ${testes_log}/${arquivo3} ]; then
    echo "O arquivo ${arquivo3} não existe. Executando o código utilizando o MPI com o tamanho ${tamanho} e ${num_processos} processos MPI"
    comando="time mpirun -np ${num_processos} $programa_mpi -d ${tamanho} -D 2> ${testes_log}/${arquivo3}"

    echo $comando

    eval $comando
fi

arquivo4="saida_mpi_pthreads_${tamanho}_${num_processos}_${num_threads}.out"
if [ ! -e ${testes_log}/${arquivo4} ]; then
    echo "O arquivo ${arquivo4} não existe. Executando o código pthreads + MPI com o tamanho ${tamanho} e ${num_processos} processos MPI com ${num_threads} threads cada"
    comando="time mpirun -np ${num_processos} $programa_pthread_mpi -d ${tamanho} -t ${num_threads} -D 2> ${testes_log}/${arquivo4}"

    echo $comando

    eval $comando
fi

if [ -e "${testes_log}/${arquivo1}" ] && [ -e "${testes_log}/${arquivo2}" ] && [ -e "${testes_log}/${arquivo3}" ] && [ -e "${testes_log}/${arquivo4}" ]; then
    echo "Todos os quatro arquivos existem. Executando o diff de cada saida do "

    diff "${testes_log}/${arquivo1}" "${testes_log}/${arquivo2}" > "${testes_log}/diff_1_2.txt"

    diff "${testes_log}/${arquivo1}" "${testes_log}/${arquivo3}" > "${testes_log}/diff_1_3.txt"

    diff "${testes_log}/${arquivo1}" "${testes_log}/${arquivo4}" > "${testes_log}/diff_1_4.txt"

    if [ -s "${testes_log}/diff_1_2.txt" ]; then
        echo "Diferenças encontradas entre ${arquivo1} e ${arquivo2}:"
        cat "${testes_log}/diff_1_2.txt"
    else
        echo "Nenhuma diferença encontrada entre ${arquivo1} e ${arquivo2}."
    fi

    if [ -s "${testes_log}/diff_1_3.txt" ]; then
        echo "Diferenças encontradas entre ${arquivo1} e ${arquivo3}:"
        cat "${testes_log}/diff_1_3.txt"
    else
        echo "Nenhuma diferença encontrada entre ${arquivo1} e ${arquivo3}."
    fi

    if [ -s "${testes_log}/diff_1_4.txt" ]; then
        echo "Diferenças encontradas entre ${arquivo1} e ${arquivo4}:"
        cat "${testes_log}/diff_1_4.txt"
    else
        echo "Nenhuma diferença encontrada entre ${arquivo1} e ${arquivo4}."
    fi

    rm -f "${testes_log}/diff_1_2.txt" "${testes_log}/diff_1_3.txt" "${testes_log}/diff_1_4.txt"

else
    echo "Alguns arquivos não existem, o diff não será executado."
fi