#!/bin/bash

## input
label=${1}
outdir=${2}
dataset=${3}
era=${4}
version=${5}

eosbase="/store/user/kmcdermo"
eosdir="${eosbase}/${dataset}/${label}_Data_Run2017${era}-PromptReco-${version}"
timestamp=$(eos ls ${eosdir})
eos ls ${eosdir}/${timestamp}/0000/ > tmp.txt

grep ".root" tmp.txt > ${outdir}/${dataset}_2017${era}-${version}_files.txt
rm tmp.txt
