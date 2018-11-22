#!/bin/bash

## source first
source scripts/common_variables.sh

## Command Line Input
outdir=${1:-"madv2_v1/checks_v2/dumps"}

## 2D plot config
plot="met_vs_time"
misc2D="empty"

## Make 2D Input Plots + Cross Contamination
for input in "${inputs[@]}"
do 
    ## for now, do not want to know how the SR is really doing
    if [[ "$input" == "SR" ]]; then
	continue
    fi

    echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	## tmp out name
	outtext="${plot}_${label}"

	## make plot
	./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc2D}.${inTextExt}" "${MainEra}" "${outtext}" "${outdir}"

	## dump cross contamination
	./scripts/runXContaminationDumper.sh "${outtext}.root" "${xcontdumpconfigdir}/${plot}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${MainEra}" "${outtext}" "${outdir}"
    done
done

## Final Message ##
echo "Finished making cross contamination logs and plots"
