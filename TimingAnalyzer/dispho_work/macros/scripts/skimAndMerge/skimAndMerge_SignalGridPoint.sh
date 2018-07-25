#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
model=${2}
gridpoint=${3}
extra=${4:-""}

## global vars
mcbase="${model}"
text="${mcbase}_${gridpoint}"

## directories needed
indir="${inbase}/analysis/${mcbase}_${gridpoint}${extra}/${label}_${mcbase}_${gridpoint}${extra}"
tmpdir="${tmpbase}/${mcbase}/${gridpoint}"
outdir="${outbase}/MC/${mcbase}/${gridpoint}"

./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} 1
