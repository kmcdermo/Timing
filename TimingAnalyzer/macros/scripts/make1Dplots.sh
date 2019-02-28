#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v2"}
plotlist=${2:-"${reducedplotlist}"}
save_meta_data=${3:-0}

## main loops
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel
    do
	nohup ./scripts/make1DplotsSubRoutine.sh "${outdir}" "${plotlist}" ${save_meta_data} "${input}" "${label}" "${infile}" "${insigfile}" "${sel}" >& "${input}_1Dplots.log" &
    done
done

## final message
echo "Finished Making1DPlots"
