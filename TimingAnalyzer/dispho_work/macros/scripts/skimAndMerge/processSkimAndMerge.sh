#!/bin/bash

source scripts/common_variables.sh

## input
text=${1}

indir=${2}
tmpdir=${3}
outdir=${4}

## global vars
files="${text}_files.txt"
tmpfiles="tmp_${files}"
timestamp=$(eos ls ${indir})
eosdir="${indir}/${timestamp}/0000"

## holla back
echo "Processing:" ${text} 

## first get list of files
eos ls ${eosdir} > ${tmpfiles}
grep ".root" ${tmpfiles} > ${files}
rm ${tmpfiles}

## make tmp dir
mkdir -p ${tmpdir}

## read in each file and skim
nfiles=$(wc -l ${files})
counter="1"
while IFS='' read -r line || [[ -n "${line}" ]]; do
    echo "Working on file" ${counter} "out of" ${nfiles} "[filename: ${line}]"
    ./scripts/runSkimmer.sh ${rootbase}/${eosdir} ${tmpdir} ${line}
    counter=$((${counter} + 1))
done < "${files}"

## remove list of files
rm ${files}

## Hadd on tmp 
hadd -O -k ${tmpdir}/${outfile} ${tmpdir}/${infiles}
rm -rf ${tmpdir}/${infiles}

## Copy back to EOS
eos mkdir -p ${outdir}
xrdcp -r ${tmpdir}/${outfile} ${rootbase}/${outdir}
rm -rf ${tmpdir}/${outfile}
