#!/bin/bash

for sel in gjets
do
    for plot in njetsidL jetHTidL njetsidT jetHTidT phopt_0 phoeta_0 phophi_0 phoseedtime_0 met met_slim phopt_1
    do
	./scripts/runTreePlotter.sh "cut_config/control_${sel}.txt" "plot_config/${plot}.txt" ${plot} "plots/${sel}_wpixveto_noidpho1_jetpt0gt50"
    done
done
