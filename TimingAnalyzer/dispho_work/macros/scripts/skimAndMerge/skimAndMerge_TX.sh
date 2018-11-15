#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
sample=${2}
tune=${3}

## other input vars
usePUWeights=1

## global vars
if [[ "${sample}" == *"ttZ"* ]]
then
    extra=$( echo "${sample}" | cut -d "Z" -f 2)
    mcbase="TTZ${extra}"
elif [[ "${sample}" == *"ttW"* ]]
then
    extra=$( echo "${sample}" | cut -d "W" -f 2)
    mcbase="TTW"
else
    mcbase="${sample}"
fi

text="${mcbase}"

## directories needed
indir="${inbase}/${sample}_${tune}/${label}_${sample}_${tune}"
tmpdir="${tmpbase}/${mcbase}"
outdir="${outbase}/MC/TX/${mcbase}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${usePUWeights}
