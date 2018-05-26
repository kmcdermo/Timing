#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks"}
insel="always_true"

for inputgroup in "qcd signals_qcd control_qcd SinglePhoton" "gjets signals_gjets control_gjets DoubleEG" "sr signals_sr signal SinglePhoton"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in njets30 jetHT30 jetHT30_zoom nvtx sumET sumET_zoom phopt_0 phopt_0_zoom phopt_1 phopt_1_zoom phophi_0 phoeta_0 phosmaj_0 phosmin_0
	    do
	    misc="misc"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
	done
    done
done

for inputgroup in "qcd signals_qcd control_qcd SinglePhoton" "gjets signals_gjets control_gjets DoubleEG"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in phoseedtime_0 met met_zoom
	do
	    misc="misc"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
	done
    done
done

for inputgroup in "sr signals_sr signal SinglePhoton"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in phoseedtime_0 met met_zoom
	do
	    misc="misc_blind"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
	done
    done
done
