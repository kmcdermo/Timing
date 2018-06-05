#!/bin/bash

outdir=${1:-"plots/ntuples_v4/signals_only"}
insel="met200"
extra="_L200"

for inputgroup in "qcd signals_qcd control_qcd SinglePhoton" #"gjets signals_gjets control_gjets DoubleEG" #
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in njets30 jetHT30 jetHT30_zoom nvtx vtxX vtxY vtxZ sumET sumET_zoom phopt_0 phopt_0_zoom phopt_1 phopt_1_zoom phophi_0 phoeta_0 phosmaj_0 phosmin_0 jetpt_0 jetpt_0_zoom jetphi_0 jeteta_0
	    do
	    misc="misc_signals_L200"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}${extra}"
	done
    done
done

for inputgroup in "qcd signals_qcd control_qcd SinglePhoton" #"gjets signals_gjets control_gjets DoubleEG" #
do echo ${inputgroup} | while read -r infile insignalfile sel pdname
    do
	for plot in phoseedtime_0 met met_zoom
	do
	    misc="misc_signals_L200"
	    ./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${insel}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_${sel}_${pdname}" "${outdir}/${sel}_${insel}${extra}"
	done
    done
done
