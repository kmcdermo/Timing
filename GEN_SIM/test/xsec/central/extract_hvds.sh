#!/bin/bash

tmpoutput="hvds_xsecs.txt"
> ${tmpoutput}

for MZp in 300 500 800 1000
do
    for MDP in 20 40 60
    do
	for ctau in 1 100 500 1000 2500 5000 10000
	do
	    xsec=$(  grep "final cross section" xsec_HVDS_MZp-${MZp}_MDP-${MDP}_Ctau-${ctau}mm_TuneCP5_13TeV-pythia8.log | cut -d " " -f 7 )
	    exsec=$( grep "final cross section" xsec_HVDS_MZp-${MZp}_MDP-${MDP}_Ctau-${ctau}mm_TuneCP5_13TeV-pythia8.log | cut -d " " -f 9 )
	
	    echo ${MZp} ${MDP} ${ctau} ${xsec} ${exsec} >> ${tmpoutput}
	done
    done
done