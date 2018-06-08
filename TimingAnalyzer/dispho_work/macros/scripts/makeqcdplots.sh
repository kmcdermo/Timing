#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks_v2"}
insel_dir="cuts_v2/"
insel="njetsGT3"

for inputgroup in "qcd signals_qcd control_qcd SinglePhoton"
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

for inputgroup in "qcd signals_qcd control_qcd SinglePhoton"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in phoseedtime_0 met met_zoom
	do
	    misc="misc"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel_dir}${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}"
	done
    done
done
