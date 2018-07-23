#!/bin/bash

outdir=${1:-"plots"}
plot="met_vs_time"
misc2D="empty"

## input configs
CR_GJets="gjets signals_gjets always_true gjets_phopt_0_map cr_gjets_DEG"
CR_QCD="qcd signals_qcd cuts_v3/invertiso0_v0 qcd_phopt_0_map cr_qcd_DEG"
SR="sr signals_sr always_true empty sr_SPH"
declare -a inputs=(CR_GJets CR_QCD SR)

## make input plots
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r infile insigfile sel varwgtmap label
    do
	./scripts/runTreePlotter2D.sh "skims/${infile}.root" "skims/${insignalfile}.root" "cut_config/${sel}.txt" "plot_config/${plot}.txt" "misc_config/${misc2D}.txt" "${plot}_${label}" "${outdir}"
    done
done

## run fitter over input plots
./scripts/runFitter.sh



## copy to combine --> comment out for bias study
combinedir="combine/input"
mkdir -p ${combinedir}
cp ${outfiletext}.root ${combinedir} 
