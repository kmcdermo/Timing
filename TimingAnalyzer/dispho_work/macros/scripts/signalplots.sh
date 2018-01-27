#!/bin/bash

## do full cuts first, no blinding
for sel in nohlt hlt
do
    for plot in phopt_0 phoeta_0 phophi_0
    do
	./scripts/runTreePlotter.sh "cut_config/signal_unblinded_${sel}.txt" "plot_config/${plot}.txt" ${plot} "plots/${sel}"
    done
done

## do seedtime (blinded)
for sel in nohlt hlt
do
    for plot in phoseedtime_0
    do
	./scripts/runTreePlotter.sh "cut_config/signal_blindedtime_${sel}.txt" "plot_config/${plot}.txt" ${plot} "plots/${sel}"
    done
done

## do MET (blinded)
for sel in nohlt hlt
do
    for plot in met
    do
	./scripts/runTreePlotter.sh "cut_config/signal_blindedmet_${sel}.txt" "plot_config/${plot}.txt" ${plot} "plots/${sel}"
    done
done

#HT="jetHTeta3 HT 100 0 5000 ${logaxis} HT H_{T}XXX[GeV],XXXjetXXXp_{T}>15XXXGeV/c ${ytitle} ${delim}"
#njets="njets njets_pt15 25 0 25 ${logaxis} njets nJetsXXX[p_{T}>15XXXGeV/c] ${ytitle} ${delim}"
