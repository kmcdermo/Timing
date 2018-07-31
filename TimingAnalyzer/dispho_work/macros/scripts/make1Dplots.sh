#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"plots/ntuples_v4/checks_v2"}

## main loops
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	while IFS='' read -r plot || [[ -n "${plot}" ]];
	do
	    if [[ ${plot} != "" ]];
	    then
		## output filename
		outfile="${plot}_${label}"

		## determine which misc file to use
		misc=$(GetMisc ${input} ${plot})

		## vargtmap == empty for data_mc
		varwgtmap="empty"

		## run script
		./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${outfile}" "${outdir}/${label}"
	    fi
	done < "${plotconfigdir}/${standardplotlist}.${inTextExt}"
    done
done

## Final message
echo "Finished Making1DPlots"
