#!/bin/bash

## inputs
infile=${1:-"abcd_scan_for_r.log"}

## outputs
outdir=${2:-"output"}
mkdir -p "${outdir}"

while IFS='' read -r line || [[ -n "${line}" ]] 
do
    echo "${line}" | while read -r sample best_dir
    do
	echo "Working on ${sample}"
	cp "${best_dir}/limits/AsymLim${sample}.root" "${outdir}"
    done
done < "${infile}"

