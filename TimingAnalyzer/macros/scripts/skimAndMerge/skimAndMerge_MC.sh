#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
sample=${2}
mcbase=${3}
dir=${4}

## other input vars
usePUWeights=1

## global vars
text="${mcbase}_${sample}"

## directories needed
indir="${inbase}/${sample}/${label}_${sample}"
tmpdir="${tmpbase}/${mcbase}/${sample}"
outdir="${outbase}/MC/${mcbase}/${dir}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${usePUWeights}
