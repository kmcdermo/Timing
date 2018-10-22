#!/bin/bash

tmpoutput="gmsb.txt"
> "${tmpoutput}"

for lambda in 100 150 200 250 300 350 400 500 600
do
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000
    do
	if [[ "${ctau}" == "0p1" ]]
	then
	    if [[ "${lambda}" == "500" ]] || [[ "${lambda}" == "600" ]] 
	    then
		ctau_s=${ctau}
	    else
		ctau_s="0_1"
	    fi
	else
	    ctau_s=${ctau}
	fi

	xsec=$( grep "${lambda} ${ctau} " gmsb_xsecs.txt | cut -d " " -f 3)
	br=$( grep "${lambda} ${ctau} " all_params.txt | cut -d " " -f 5)

	echo "['/GMSB_L-${lambda}TeV_Ctau-${ctau_s}cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '${xsec}', '1', '${br}']," >> "${tmpoutput}"
    done
done
