#!/bin/bash

outdir=${1:-"plots"}
plot="met_vs_time"
insel="always_true"

for group in "qcd_skim control_qcd SinglePhoton" "gjets_skim control_gjets_deg DoubleEG" "sr_skim signal_idT_hlt_nJet3pt30_ht400jetpt30_sumEtjetpt30phopt0_hltreco SinglePhoton"
do echo ${group} | while read -r infile sel pdname
    do
	./scripts/runTreePlotter2D.sh "skims/${infile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
    done
done
