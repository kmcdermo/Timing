#!/bin/bash

source scripts/common_variables.sh

## input
insampledir=${1:-"DoubleEG/crab_deg_Run2017C-17Nov2017-v1"}
outsampledir=${2:-"Data/DoubleEG/C/v1"}

## tmp dir variables
indir="${inbase}/analysis/${insampledir}"
timestamp=$(ls ${indir})
ineosdir="${indir}/${timestamp}/0000"

## tmp files
text="unskimmed"
files="${text}_files.txt"
tmpfiles="tmp_${files}"
wgtfile="${text}_wgt.txt"

## first get list of files
echo "Getting list of unskimmed files on EOS"
ls ${ineosdir} > ${tmpfiles}
grep ".root" ${tmpfiles} > ${files}

## then run compute sum weights
echo "Getting sum of unskimmed weights"
./scripts/computeSumWeights.sh ${ineosdir} ${files} ${wgtfile}

## get tmp outdir
outeosdir="${outbase}/${outsampledir}"

## then run checker
echo "Checking sum of weights"
root -b -q -l checkSumWeights.C\(\"${wgtfile}\",\"${outeosdir}\"\)

## remove tmp files
rm ${tmpfiles}
rm ${files}
rm ${wgtfile}

## Final message
echo "Finished CheckingSumWeights"
