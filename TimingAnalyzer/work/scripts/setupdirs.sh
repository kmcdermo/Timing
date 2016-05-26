#!/bin/sh

for dir in dep bin input
do
    mkdir -p ${dir}
done

for subdir in DATA MC
do
    mkdir -p input/${subdir}
done

for subdir in doubleeg
do
    mkdir -p input/DATA/${subdir}
done

for subdir in dyll qcd gamma
do
    mkdir -p input/MC/${subdir}
done


