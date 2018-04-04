#!/bin/bash

sel=${1:-"control_gjets"}
scalearea=${2:-0}
outdir=${3:-"plots"}

for plot in phopt_0 phopt_0_zoom phoseedtime_0 phopt_1 phopt_1_zoom met met_zoom njets jetHT jetHT_zoom delphi diphomass nvtx
do
    ./scripts/runTreePlotter.sh "cut_config/${sel}.txt" "plot_config/${plot}.txt" ${scalearea} "${plot}_${sel}" "${outdir}/${sel}"
done
