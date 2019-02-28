#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
scan_log=${1:-"abcd_scan_for_r.log"}
outlimitdir=${2:-"output"}
outlimitplotdir=${3:-"limits"}
outcombname=${4:-"AsymLim"}

## Loop over scan dir to 
while IFS='' read -r line || [[ -n "${line}" ]] 
do
    echo "${line}" | while read -r sample best_dir
    do
	echo "Working on ${sample}"
	cp "${best_dir}/${outlimitplotdir}/${outcombname}${sample}.root" "${outlimitdir}"
    done
done < "${scan_log}"
