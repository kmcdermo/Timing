#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v2"}
plotlist=${2:-"standard"}
savemetadata=${3:-0}

## main loops
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel
    do
	nohup ./scripts/make1DplotsSubRoutine.sh "${outdir}" "${plotlist}" ${savemetadata} "${input}" "${label}" "${infile}" "${insigfile}" "${sel}" >& "${input}_1Dplots.log" &
    done
done

## Final message
echo "Finished Making1DPlots"
