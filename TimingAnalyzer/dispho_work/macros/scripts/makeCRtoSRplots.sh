#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks_v3/CRtoSR"}

for cr in qcd gjets
do
    for plot in phoseedtime_0 met met_zoom
    do
	./scripts/runCRtoSRPlotter.sh "crtosr_config/${cr}_${plot}.txt" "${plot}" "${outdir}/${cr}"
    done
done

## Final message
echo "Finished MakingCRtoSRPlots"
