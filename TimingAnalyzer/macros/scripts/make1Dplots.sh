#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v2"}
plotlist=${2:-"standard"}
usewgts=${3:-"true"}
savemetadata=${4:-0}

## main loops
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	nohup ./scripts/make1DplotsSubRoutine.sh "${outdir}" "${plotlist}" "${usewgts}" ${savemetadata} "${input}" "${label}" "${infile}" "${insigfile}" "${sel}" "${varwgtmap}" >& "${input}_1Dplots.log" &
    done
done

## Final message
echo "Finished Making1DPlots"
