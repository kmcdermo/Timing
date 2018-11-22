#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"met_vs_time.root"}
xcontdumpconfig=${2:-"${xcontdumpconfigdir}/met_vs_time.${inTextExt}"}
plotconfig=${3:-"${plotconfigdir}/met_vs_time.${inTextExt}"}
era=${4:-"Full"}
outfiletext=${5:-"met_vs_time"}
dir=${6:-"plots"}

## 2D plots
declare -a plots=("full" "block1D" "block2D")
declare -a events=("int" "frac")

## first make plot
root -l -b -q runXContaminationDumper.C\(\"${infilename}\",\"${xcontdumpconfig}\",\"${plotconfig}\",\"${era}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
for plot in "${plots[@]}"
do
    for event in "${events[@]}"
    do
	for ext in "${exts[@]}"
	do
	    cp ${outfiletext}_${plot}_${event}.${ext} ${fulldir}
	done
    done
done

cp ${outfiletext}.root ${outfiletext}.${outTextExt} ${fulldir}

## Final message
echo "Finished XContaminationDumping for plot:" ${infilename}
