#!/bin/bash

source scripts/common_variables.sh

## input
text=${1}

indir=${2}
tmpdir=${3}
outdir=${4}

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
sumwgts=$(grep "Sum of weights: " ${wgtfile} | cut -d " " -f 4)

## read in each file and skim
echo "Running skimming macro"
nfiles=$(wc -l ${files})
counter="1"
while IFS='' read -r line || [[ -n "${line}" ]]; do
    echo "Working on file" ${counter} "out of" ${nfiles} "[filename: ${line}]"
    ./scripts/runSkimmer.sh ${rootbase}/${eosdir} ${tmpdir} ${line} ${sumwgts}
    counter=$((${counter} + 1))
done < "${files}"

## remove txt files
echo "Removing txt files"
rm ${wgtfile}
rm ${files}

## Hadd on tmp 
echo "Hadding skims on tmp and then removing individual skim files"
hadd -O -k ${tmpdir}/${outfile} ${tmpdir}/${infiles}
rm -rf ${tmpdir}/${infiles}

## Copy back to EOS
echo "Copy hadded skim to EOS"
eos mkdir -p ${outdir}
xrdcp -r ${tmpdir}/${outfile} ${rootbase}/${outdir}
rm -rf ${tmpdir}/${outfile}
