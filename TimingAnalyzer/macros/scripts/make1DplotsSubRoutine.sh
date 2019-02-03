#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v2"}
plotlist=${2:-"standard"}
usewgts=${3:-"true"}
savemetadata=${4:-0}

## derived config from input
input=${5:-""}
label=${6:-""}
infile=${7:-""}
insigfile=${8:-""}
sel=${9:-""}
varwgtmap=${10:-""}

## check for no weights
if [[ "${usewgts}" == "false" ]]; then
    varwgtmap="empty"
fi

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
	./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${MainEra}" ${savemetadata} "${outfile}" "${outdir}/${label}"
    fi
done < "${plotconfigdir}/${plotlist}.${inTextExt}"

## Final message
echo "Finished Making1DPlotsSubRoutine"
