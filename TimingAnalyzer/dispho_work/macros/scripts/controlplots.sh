#!/bin/bash

for sel in  qcd
do
    for plot in phopt_0 phoeta_0 phophi_0 phoseedtime_0 met njetseta3 jetHTeta3
    do
	./scripts/runTreePlotter.sh "cut_config/control_${sel}.txt" "plot_config/${plot}.txt" ${plot} "plots/control_${sel}"
    done
done