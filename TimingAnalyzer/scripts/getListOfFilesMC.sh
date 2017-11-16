#!/bin/bash

## input
label=${1}
outdir=${2}
dataset=${3}
tune=${4}

eosbase="/store/user/${USER}"
eosdir="${eosbase}/${dataset}_${tune}/${label}_${dataset}_${tune}"
timestamp=$(eos ls ${eosdir})
eos ls ${eosdir}/${timestamp}/0000/ > tmp.txt

grep ".root" tmp.txt > ${outdir}/${dataset}_files.txt
rm tmp.txt
