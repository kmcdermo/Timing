#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
bin=${2}

## global vars
mcbase="GJets_HT"
text="${mcbase}_${bin}"
tmptxt="${text}_files.txt"

## directories needed
indir="${eosbase}/${mcbase}-${bin}_${bkgdtune}/${label}_${mcbase}-${bin}_${bkgdtune}"
tmpdir="${tmpbase}/${mcbase}/${bin}"
outdir="${outbase}/MC/${mcbase}/${bin}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${tmptxt}
