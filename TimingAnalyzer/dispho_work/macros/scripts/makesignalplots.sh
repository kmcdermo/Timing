#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks_v3"}
insel_dir=""
insel="always_true"
varwgtmap="empty"

extra=""

for inputgroup in "sr signals_sr signal SinglePhoton misc"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname misc
    do
	while IFS='' read -r plot || [[ -n "${plot}" ]]; 
	do
	    if [[ ${plot} != "" ]];
	    then
		./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel_dir}${insel}.txt" "varwgt_config/${varwgtmap}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}${extra}"
	    fi
	done < plot_config/standard_plots.txt
    done
done

for inputgroup in "sr signals_sr signal SinglePhoton misc_blind"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname misc
    do
	for plot in phoseedtime_0 met met_zoom
	do
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel_dir}${insel}.txt" "varwgt_config/${varwgtmap}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}${extra}"
	done
    done
done

## Final message
echo "Finished MakingSignalPlots"
