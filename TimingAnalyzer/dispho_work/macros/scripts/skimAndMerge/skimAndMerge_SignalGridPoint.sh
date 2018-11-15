#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
model=${2}
gridpoint=${3}
tune=${4}

## other input vars
usePUWeights=1

## global vars
mcbase="${model}"
text="${mcbase}_${gridpoint}"

## directories needed
indir="${inbase}/${mcbase}_${gridpoint}_${tune}/${label}_${mcbase}_${gridpoint}_${tune}"
tmpdir="${tmpbase}/${mcbase}/${gridpoint}"
outdir="${outbase}/MC/${mcbase}/${gridpoint}"

./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${usePUWeights}
