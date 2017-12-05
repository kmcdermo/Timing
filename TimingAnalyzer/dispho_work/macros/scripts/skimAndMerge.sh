#!/bin/bash

## input
indir=${1}
tmpdir=${2}
outdir=${3}
tmptxt=${4}

## read in each file and skim
while IFS='' read -r line || [[ -n "${line}" ]]; do
    ./scripts/runSkimmer.sh ${indir} ${tmpdir} ${line}
done < "${tmptext}"

## Hadd on tmp 
hadd -O -k ${tmpdir}/${outfile} ${tmpdir}/${infiles}
rm -rf ${tmpdir}/${infiles}

## Copy back to EOS
eos mkdir -p ${outdir}
xrdcp -r ${tmpdir}/${outfile} ${rootbase}/${outdir}/
rm -rf ${tmpdir}/${outfile}
