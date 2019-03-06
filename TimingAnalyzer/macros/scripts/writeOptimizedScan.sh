#!/bin/bash

## source first
source scripts/common_variables.sh

## config
indir=${1:-"madv2_v3/checks_v26"}

## derived config
fulldir="${topdir}/${disphodir}/${indir}"

## make scan log
> "${scan_log}"

## optimized bins
declare -a bins=("2 500" "0.5 200" "1.5 200")

## loop lambda and ctau
for lambda in 100 150 200 250 300 350 400
do
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000
    do
	sample="GMSB_L${lambda}_CTau${ctau}"
	bin=""

	if [[ "${ctau}" == "0p001" ]] || [[ "${ctau}" == "0p1" ]]
	then
	    bin="${bins[0]}"
	elif [[ "${ctau}" == "10" ]]
	then
	    bin="${bins[1]}"
	else
	    bin="${bins[2]}"
	fi

	x=$( echo "${bin}" | cut -d " " -f 1 )
	y=$( echo "${bin}" | cut -d " " -f 2 )
	echo "${sample} ${fulldir}/x_${x}_y_${y}" >> "${scan_log}"
    done
done
