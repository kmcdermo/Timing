#!/bin/bash

sel=${1:-"control_gjets"}
outdir=${2:-"plots"}

for plot in phopt_0 phoeta_0 phophi_0 phoseedtime_0 phosieie_0 phoHoE_0 phosmaj_0 phosmin_0 phoEcalPFClIso_0 phoHcalPFClIso_0 phoTrkIso_0 phopt_1 diphomass delphi njetsidL jetHTidL njetsidT jetHTidT njetsnpho jetHTnopho met met_slim
do
    ./scripts/runTreePlotter.sh "cut_config/${sel}.txt" "plot_config/${plot}.txt" "${plot}_${sel}" "${outdir}/${sel}"
done
