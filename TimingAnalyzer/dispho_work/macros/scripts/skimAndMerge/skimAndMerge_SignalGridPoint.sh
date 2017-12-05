#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
model=${2}
gridpoint=${3}

## global vars
mcbase="${model}"
text="${mcbase}_${gridpoint}"

## directories needed
indir="${eosbase}/${mcbase}_${gridpoint}/${label}_${mcbase}_${gridpoint}"
tmpdir="${tmpbase}/${mcbase}/${gridpoint}"
outdir="${outbase}/MC/${mcbase}/${gridpoint}"

./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir}
