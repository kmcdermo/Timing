#!/bin/bash

crtosrconfig=${1:-"crtosr_config/qcd_phoseedtime_0.txt"}
outfiletext=${2:-"qcd_phoseedtime_0"}
dir=${3:-"plots/ntuples_v4/checks_v3/CRtoSR"}

## first make plot
root -l -b -q runCRtoSRPlotter.C\(\"${crtosrconfig}\",\"${outfiletext}\"\)

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
for plotscale in norm scaled
do
    for canvscale in log lin
    do
	cp ${outfiletext}_${plotscale}_${canvscale}.png ${fulldir}
    done
done

## Final message
echo "Finished CRtoSRPlotting"
