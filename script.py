import subprocess
import sys
from decimal import Decimal
import numpy as np


def main():
    matrix_size = sys.argv[1]
    threads = int(sys.argv[2])
    values = []
    for i in range(10):
        if threads == 0:
            value = subprocess.check_output(
                f"./syr2k -d {matrix_size}", shell=True)
        else:
            value = subprocess.check_output(
                f"./syr2k_paralelo -d {matrix_size} -t {threads} ", shell=True)
        string_value = value.decode("utf-8")
        aux = string_value.split("\n")
        aux = aux[0].split("sec")
        aux = aux[0].split(": ")
        value = Decimal(aux[1])
        print(f"{i}: {value}")
        values.append(value)
    media = sum(values)/10
    desvio = np.std(values)
    print("")
    print(f"media: {media} desvio padrão: {desvio}")


if len(sys.argv) != 3:
    print(f"Uso: {sys.argv[0]} <matriz_size> <threads>")
else:
    main()
