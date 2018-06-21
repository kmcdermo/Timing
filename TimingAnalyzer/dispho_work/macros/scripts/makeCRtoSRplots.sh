#!/bin/bash

outdir=${1:-"plots/ntuples_v4/checks_v3/jetclean_checks/CRtoSR"}

for cr in qcd gjets
do
    while IFS='' read -r line || [[ -n "${line}" ]]; 
    do
	if [[ ${line} != "" ]];
	then
	    plot=${line}
	    ./scripts/runCRtoSRPlotter.sh "crtosr_config/${cr}_${plot}.txt" "${cr}_${plot}" "${outdir}/${cr}"
	fi
    done < crtosr_config/standard_plots.txt
done

## Final message
echo "Finished MakingCRtoSRPlots"
