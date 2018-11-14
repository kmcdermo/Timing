#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
sample=${2}
tune=${3}

## other input vars
usePUWeights=1

## global vars
mcbase="${sample}"
text="${mcbase}"

## directories needed
indir="${inbase}/${sample}_${tune}/${label}_${sample}_${tune}"
tmpdir="${tmpbase}/${mcbase}"
outdir="${outbase}/MC/ZX/${mcbase}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${usePUWeights}
