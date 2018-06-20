#!/bin/bash

srplotconfig=${1:-"srplot_config/phoseedtime_0.txt"}
miscconfig=${2:-"misc_config/misc_blind.txt"}
outfiletext=${3:-"phoseedtime_0_srplot"}
dir=${4:-"plots/ntuples_v4/checks_v3/SRPlot"}

## first make plot
root -l -b -q runSRPlotter.C\(\"${srplotconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

## copy everything
cp ${outfiletext}_log.png ${outfiletext}_lin.png ${outfiletext}.root ${outfiletext}_integrals.txt ${fulldir}

## Final message
echo "Finished SRPlotting"
