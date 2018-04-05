#!/bin/bash

outdir=${1:-"plots"}
plot="phophi_0"
scalearea=0

for sel in control_qcd nojetpt0 nonjets nomass nophopt1 nophoID0 nohlt
do
    ./scripts/runTreePlotter.sh "cut_config/${sel}.txt" "plot_config/${plot}.txt" ${scalearea} "${plot}_${sel}" "${outdir}"
done
