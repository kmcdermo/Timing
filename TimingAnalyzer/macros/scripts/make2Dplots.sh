#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"plots2D"}
savemetadata=${2:-0}
plot="met_vs_time"
misc="empty"

## main loop
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	## make outfile text
	outfile="${plot}_${label}"

	## run script
	./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${MainEra}" ${savemetadata} "${outfile}" "${outdir}/${label}"
    done
done

## Final message
echo "Finished Making2DPlots"
