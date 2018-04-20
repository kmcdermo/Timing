#!/bin/bash

outdir=${1:-"plots"}
plot="met_vs_time"

for group in "control_qcd SinglePhoton" "control_gjets_deg DoubleEG" "signal_idT_hlt_nJet3pt30_ht400jetpt30_sumEtjetpt30phopt0_hltreco SinglePhoton"
do echo ${group} | while read -r sel pdname
    do
	./scripts/runTreePlotter2D.sh "cut_config/${sel}.txt" "plot_config/${plot}.txt" "${pdname}" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
    done
done
