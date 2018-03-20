#!/bin/bash

sel=${1:-"control_gjets"}

for plot in phopt_0 phoeta_0 phophi_0 phoseedtime_0 phosieie_0 phoHoE_0 phosmaj_0 phosmin_0 phoEcalPFClIso_0 phoHcalPFClIso_0 phoTrkIso_0 phopt_1 diphomass met met_slim njetsidL jetHTidL njetsidT jetHTidT
do
    ./scripts/runTreePlotter.sh "cut_config/${sel}.txt" "plot_config/${plot}.txt" "${plot}_${sel}" "plots/full/${sel}"
done
