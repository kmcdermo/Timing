#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v2"}
plotlist=${2:-"standard"}
save_meta_data=${3:-0}

## derived config from input
input=${4:-""}
label=${5:-""}
infile=${6:-""}
insigfile=${7:-""}
sel=${8:-""}

## loop over plots
while IFS='' read -r plot || [[ -n "${plot}" ]];
do
    if [[ ${plot} != "" ]];
    then
	## output filename
	outfile="${plot}_${label}"
	
	## determine which misc file to use
	misc=$( GetMisc ${input} ${plot} )
	
	## run script
	./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${MainEra}" ${save_meta_data} "${outfile}" "${outdir}/${label}"
    fi
done < "${plotconfigdir}/${plotlist}.${inTextExt}"

## Final message
echo "Finished Making1DPlotsSubRoutine"
