#!/bin/bash

tmpoutput="all_params.txt"
> "${tmpoutput}"

for lamb in 100 150 200 250 300 350 400 500 600
do
    for ctau in 0.001 0.01 0.1 0.5 1 5 10 50 100 200 400 600 800 1000 1200 5000 10000
    do
	slha=SLHA/GMSB_Lambda${lamb}TeV_CTau${ctau}cm.slha
	mass=$( grep -m 1 "1000022" ${slha} | cut -d ' ' -f9 )
	br=$( grep "Z1SS   -->  GVSS   GM" ${slha} | cut -d ' ' -f7 )
	width=$( grep "DECAY   1000022" ${slha} | cut -d ' ' -f6 )

	diffs=Diffs/Lambda${lamb}TeV_diffs.txt
	genctau=$( grep -w "^${ctau}" ${diffs} | cut -d ' ' -f2 )

	echo "${lamb} ${ctau} ${genctau} ${mass} ${width} ${br}" >> "${tmpoutput}"
    done
done
