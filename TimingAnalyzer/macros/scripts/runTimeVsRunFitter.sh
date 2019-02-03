#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"${skimdir}/sr.root"}
plotconfig=${2:-"${plotconfigdir}/phopt_0.${inTextExt}"}
timefitconfig=${3:-"time.${inTextExt}"}
savemetadata=${4:-0}
outfiletext=${5:-"plots"}
dir=${6:-"test"}

declare -a outputs=("mu" "sigma") ## can add back chi2prob, chi2ndf

## first make plot
root -l -b -q runTimeVsRunFitter.C\(\"${infilename}\",\"${plotconfig}\",\"${timefitconfig}\",${savemetadata},\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
for canvscale in "${canvscales[@]}"
do
    for ext in "${exts[@]}"
    do
	for output in "${outputs[@]}"
	do
	    if [[ "${canvscale}" == "log" ]] && [[ "${output}" == "mu" ]]
	    then
		continue ## do not produce logy plots for mu
	    fi

	    cp ${output}_${outfiletext}_${canvscale}.${ext} ${fulldir}
	done
    done
done
cp ${outfiletext}.root ${outfiletext}"_fitinfo".${outTextExt} ${fulldir}

## Final message
echo "Finished TimeVsRunFitting for plot:" ${plotconfig}
