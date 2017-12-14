#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}

## global vars
mcbase="GJet"
bin="Pt-15to6000"
flattune="TuneCUETP8M1-Flat_13TeV_pythia8"
text="${mcbase}_${bin}"

## directories needed
indir="${eosbase}/${mcbase}_${bin}_${flattune}/${label}_${mcbase}_${bin}_${flattune}"
tmpdir="${tmpbase}/${mcbase}/${bin}"
outdir="${outbase}/MC/${mcbase}/${bin}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir}
