#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v3/sig_effs"}

## make signal efficiency
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel
    do
	./scripts/runSigEffPlotter.sh "${skimdir}/${insigfile}.root" "sig_eff_${label}" "${outdir}"
    done
done
