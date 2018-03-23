#!/bin/bash

source scripts/common_variables.sh

## input
text=${1}

indir=${2}
tmpdir=${3}
outdir=${4}

redophoid=${redoPhotonID}

## global vars
files="${text}_files.txt"
wgtfile="${text}_wgt.txt"
tmpfiles="tmp_${files}"
timestamp=$(eos ls ${indir})
eosdir="${indir}/${timestamp}/0000"

## holla back
echo "Processing:" ${text} 

## first get list of files
echo "Getting list of files on EOS"
eos ls ${eosdir} > ${tmpfiles}
grep ".root" ${tmpfiles} > ${files}
rm ${tmpfiles}

## make tmp dir
echo "Making tmp dir"
mkdir -p ${tmpdir}

## produce sum of weights
echo "Getting sum of weights"
./scripts/runSumWeights.sh ${rootbase}/${eosdir} ${files} ${wgtfile}

rm ${files}