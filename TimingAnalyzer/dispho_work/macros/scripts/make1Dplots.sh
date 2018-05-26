#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks"}
plot="met_vs_time"
insel="always_true"
misc="empty.txt"

for inputgroup in "qcd signals_qcd control_qcd SinglePhoton" "gjets signals_gjets control_gjets_deg DoubleEG" "sr signals_sr signal_idT_hlt_nJet3pt30_ht400jetpt30_sumEtjetpt30phopt0_hltreco SinglePhoton"
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in njets30 jetHT30 jetHT30_zoom nvtx sumEt sumEt_zoom phopt_0 phopt_0_zoom phopt_1 phopt_1_zoom phophi_0 phoeta_0 phosmaj_0 phosmin_0
	    do
	    misc="misc.txt"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
	done
	
	for plot in phoseedtime_0 met met_zoom
	do
	    misc="misc_blind.txt"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
	done
    done
done
