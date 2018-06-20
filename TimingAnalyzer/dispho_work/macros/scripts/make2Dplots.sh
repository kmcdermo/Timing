#!/bin/bash

outdir=${1:-"plots"}
plot="met_vs_time"
insel="always_true"
misc="empty.txt"

for group in "qcd signals_qcd control_qcd SinglePhoton" "gjets signals_gjets control_gjets DoubleEG" "sr signals_sr signal SinglePhoton"
do echo ${group} | while read -r infile insignalfile sel pdname
    do
	./scripts/runTreePlotter2D.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
    done
done

## Final message
echo "Finished Making2DPlots"
