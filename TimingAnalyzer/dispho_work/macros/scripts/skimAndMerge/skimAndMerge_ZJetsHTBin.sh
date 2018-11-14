#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
tune=${2}
bin=${3}

## other input vars
usePUWeights=1

## global vars
mcbase="ZJetsToNuNu_HT"
text="${mcbase}_${bin}"

## directories needed
indir="${inbase}/${mcbase}-${bin}_${tune}/${label}_${mcbase}-${bin}_${tune}"
tmpdir="${tmpbase}/${mcbase}/${bin}"
outdir="${outbase}/MC/${mcbase}/${bin}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${usePUWeights}
