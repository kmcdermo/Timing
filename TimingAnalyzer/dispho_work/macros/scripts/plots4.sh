#!/bin/bash

sel=${1:-"control_gjets"}
pdname=${2:-"SinglePhoton"}
scalearea=${3:-0}
outdir=${4:-"plots"}

for plot in sumEt sumEt_zoom
do
    ./scripts/runTreePlotter.sh "cut_config/${sel}.txt" "plot_config/${plot}.txt" "${pdname}" ${scalearea} "${plot}_${sel}_${pdname}" "${outdir}/${sel}"
done
