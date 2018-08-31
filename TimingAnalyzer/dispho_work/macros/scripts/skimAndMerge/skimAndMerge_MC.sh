#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
sample=${2}
mcbase=${3}
dir=${4}
skimtype=${5:-"Standard"}

## other input vars
usePUWeights=0

## global vars
text="${mcbase}_${sample}"

## directories needed
indir="${inbase}/analysis/${sample}/${label}_${sample}"
tmpdir="${tmpbase}/${mcbase}/${sample}"
outdir="${outbase}/MC/${mcbase}/${dir}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${usePUWeights} ${skimtype}
