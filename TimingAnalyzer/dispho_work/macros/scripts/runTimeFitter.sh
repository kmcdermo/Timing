#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"${skimdir}/sr.root"}
plotconfig=${2:-"${plotconfigdir}/phopt_0.${inTextExt}"}
miscconfig=${3:-"${miscconfigdir}/misc_blind.${inTextExt}"}
timefitconfig=${4:-"time.${inTextExt}"}
era=${5:-"full"}
outfiletext=${6:-"plots"}
dir=${7:-"test"}

declare -a outputs=("mu" "sigma") ## can add back chi2prob, chi2ndf

## first make plot
root -l -b -q runTimeFitter.C\(\"${infilename}\",\"${plotconfig}\",\"${miscconfig}\",\"${timefitconfig}\",\"${era}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
for canvscale in "${canvscales[@]}"
do
    if [[ "${canvscale}" == "log" ]]
    then
	continue ## skip making logy plots for now
    fi

    for ext in "${exts[@]}"
    do
	for output in "${outputs[@]}"
	do
	    cp ${output}_${outfiletext}_${canvscale}.${ext} ${fulldir}
	    cp ${output}_${outfiletext}_${canvscale}_logx.${ext} ${fulldir}
	done
    done
done
cp ${outfiletext}.root ${fulldir}

## Final message
echo "Finished TimeFitting for plot:" ${plotconfig}
