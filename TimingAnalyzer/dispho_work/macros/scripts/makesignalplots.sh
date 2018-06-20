#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks_v2"}
insel_dir="cuts_v2/"
insel="trig_redisc_onlinevals"

for inputgroup in "sr signals_sr signal SinglePhoton"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	while IFS='' read -r line || [[ -n "${line}" ]]; 
	do
	    if [[ ${line} != "" ]];
	    then
		plot=${line}
		misc="misc"
		./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel_dir}${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}"
	    fi
	done < scripts/standard_plots.txt
    done
done

for inputgroup in "sr signals_sr signal SinglePhoton"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in phoseedtime_0 met met_zoom
	do
	    misc="misc_blind"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel_dir}${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}"
	done
    done
done

## Final message
echo "Finished MakingSignalPlots"
