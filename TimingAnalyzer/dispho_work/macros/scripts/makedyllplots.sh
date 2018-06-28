#!/bin/bash

outdir=${1:-"plots/ntuples_v4/dyll_skim"}
infile="dyll_skim"
sel="always_true"
misc="misc"

while IFS='' read -r plot || [[ -n "${plot}" ]]; 
do
    if [[ ${plot} != "" ]];
    then
	./scripts/runTreePlotter.sh "skims/${infile}.root" "cut_config/${sel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}" "${outdir}/${sel}"
    fi
done < plot_config/standard_plots.txt

## Final message
echo "Finished MakingDYLLPlots"
