#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}

## global vars
mcbase="GJet"
bin="Pt-15To6000"
flattune="TuneCP5-Flat_13TeV_pythia8"
text="${mcbase}_${bin}"

## directories needed
indir="${inbase}/ootID/${mcbase}_${bin}_${flattune}/${label}_${mcbase}_${bin}_${flattune}"
tmpdir="${tmpbase}/${mcbase}/${bin}"
outdir="${outbase}/MC/${mcbase}/${bin}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} 1
