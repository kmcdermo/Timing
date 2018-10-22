#!/bin/bash

tmpoutput="hvds.txt"
> "${tmpoutput}"

for MZp in 300 500 800 1000
do
    for MDP in 20 40 60
    do
	for ctau in 1 100 500 1000 2500 5000 10000
	do
	    xsec=$( grep "${MZp} ${MDP} ${ctau} " hvds_xsecs.txt | cut -d " " -f 4)

	echo "['/HVDS_MZp-${MZp}_MDP-${MDP}_Ctau-${ctau}mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '${xsec}', '1', '1']," >> "${tmpoutput}"
	done
    done
done