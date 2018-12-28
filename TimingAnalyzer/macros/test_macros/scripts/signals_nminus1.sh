#!/bin/bash

outdir=${1:-"plots/signals/n_minus1"}

for group in "no_pt phopt_0" "no_hlt hltSignal" "no_hltredisc hltSignal" "no_nJets njets30" "no_jetHT jetHT30" "no_jetHT jetHT30_zoom" "no_sumET sumET" "no_ID phoID_0" "sign phophi_0" "sign phoeta_0" "sign phoseedtime_0" "sign met_zoom" "sign met" "sign nvtx" "sign phoEcalPFClIso_0" "sign phoHcalPFClIso_0" "sign phoTrkIso_0" "sign phosmaj_0" "sign phosmin_0" "sign phosieie_0" "sign phoHoE_0" 
do echo ${group} | while read -r cut plot
    do
	./scripts/runTreePlotter.sh skims/sr.root skims/signals_ebonly.root cut_config/n_minus1/${cut}.txt plot_config/${plot}.txt misc_config/misc1D.txt ${plot}_${cut} ${outdir}
    done
done
