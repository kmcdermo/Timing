#!/bin/bash

outdir=${1:-"plots"}
plot="met_vs_time"
insel="always_true"
misc="empty.txt"

for group in "qcd signals_qcd control_qcd SinglePhoton" "gjets signals_gjets control_gjets_deg DoubleEG" "sr signals_sr signal_idT_hlt_nJet3pt30_ht400jetpt30_sumEtjetpt30phopt0_hltreco SinglePhoton"
do echo ${group} | while read -r infile insignalfile sel pdname
    do
	./scripts/runTreePlotter2D.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "plot_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
    done
done
