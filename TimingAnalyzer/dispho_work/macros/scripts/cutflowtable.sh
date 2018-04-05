#!/bin/bash

outdir=${1:-"plots"}
plot="phophi_0"
scalearea=0

for sel in control_qcd nojet0pt nonjets nomass nopt1 nopho0ID nohlt
do
    ./scripts/runDumpIntegrals.sh "cut_config/${sel}.txt" "plot_config/${plot}.txt" ${scalearea} "${plot}_${sel}" "${outdir}/${sel}"
done
