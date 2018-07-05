#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks_v3"}
insel_dir=""
insel="always_true"
misc="misc"
varwgtmap="empty"

for inputgroup in "gjets signals_gjets control_gjets DoubleEG"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	while IFS='' read -r plot || [[ -n "${plot}" ]]; 
	do
	    if [[ ${plot} != "" ]];
	    then
		./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel_dir}${insel}.txt" "varwgt_config/${varwgtmap}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}"
	    fi
	done < plot_config/standard_plots.txt
    done
done

for inputgroup in "gjets signals_gjets control_gjets DoubleEG"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in phoseedtime_0 met met_zoom
	do
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel_dir}${insel}.txt" "varwgt_config/${varwgtmap}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}"
	done
    done
done

## Final message
echo "Finished MakingGJetsPlots"
