#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks_v3/SRPlots"}

misc="misc_blind"
for plot in phoseedtime_0 met met_zoom
do
    ./scripts/runSRPlotter.sh "srplot_config/${plot}.txt" "misc_config/${misc}.txt" "${plot}_srplot"
done
