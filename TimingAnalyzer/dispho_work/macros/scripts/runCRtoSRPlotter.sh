#!/bin/bash

crtosrconfig=${1}
plotconfig=${2:-"plot_config/met_zoom.txt"}
outfiletext=${3:-"crtosr"}
dir=${4:-"plots/test"}

## first make plot
root -l -b -q runCRtoSRPlotter.C\(\"${crtosrconfig}\",\"${plotconfig}\",\"${outfiletext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

## copy everything
cp ${outfiletext}.root ${fulldir}
for i in norm scaled
do
    for j in log lin
    do
	cp ${outfiletext}_${i}_${j}.png ${fulldir}
    done
done