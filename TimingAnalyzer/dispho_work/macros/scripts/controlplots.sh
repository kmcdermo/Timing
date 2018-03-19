#!/bin/bash

for sel in gjets
do
    for plot in phopt_0 phoeta_0 phophi_0 phoseedtime_0 met njetsidL jetHTidL phopt_1
    do
	./scripts/runTreePlotter.sh "cut_config/control_${sel}.txt" "plot_config/${plot}.txt" ${plot} "plots/${sel}_wpixveto"
    done
done