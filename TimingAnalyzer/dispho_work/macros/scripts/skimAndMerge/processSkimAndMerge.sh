#!/bin/bash

source scripts/common_variables.sh

## input
text=${1}

indir=${2}
tmpdir=${3}
outdir=${4}

usePUWeights=${5:-0}

redophoid=${redoPhotonID}

## global vars
files="${text}_files.txt"
wgtfile="${text}_wgt.txt"
puwgtfile="${text}_puwgt.root"
tmpfiles="tmp_${files}"
timestamp=$(ls ${indir})
eosdir="${indir}/${timestamp}/0000"

## holla back
echo "Processing:" ${text} 

## first get list of files
echo "Getting list of files on EOS"
ls ${eosdir} > ${tmpfiles}
grep ".root" ${tmpfiles} > ${files}
rm ${tmpfiles}

## make tmp dir
echo "Making tmp dir"
mkdir -p ${tmpdir}

## produce sum of weights
echo "Getting sum of weights"
./scripts/computeSumWeights.sh ${eosdir} ${files} ${wgtfile}
sumwgts=$(grep "Sum of weights: " ${wgtfile} | cut -d " " -f 4)

## produce pu distribution
if (( ${usePUWeights} == 1 )) ; then
    echo "Computing PU Weights"
    ./scripts/computePUWeights.sh ${eosdir} ${files} ${puwgtfile}
fi

## read in each file and skim
echo "Running skimming macro"
nfiles=$(wc -l ${files})
counter="1"
while IFS='' read -r line || [[ -n "${line}" ]]; do
    echo "Working on file" ${counter} "out of" ${nfiles} "[filename: ${line}]"
    ./scripts/runSkimmer.sh ${eosdir} ${tmpdir} ${line} ${sumwgts} ${puwgtfile} ${redophoid}
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

## Copy to EOS
echo "Copy hadded skim to EOS"
mkdir -p ${outdir}
mv ${tmpdir}/${outfile} ${outdir}
rm -rf ${tmpdir}/${outfile}

## Copy PU weights to EOS
if (( ${usePUWeights} == 1)) ; then
    echo "Copy puwgt file to EOS"
    mv ${puwgtfile} ${outdir}/puweights.root
fi
