#!/bin/bash

srplotconfig=${1:-"srplot_config/phoseedtime_0.txt"}
miscconfig=${2:-"misc_config/misc_blind.txt"}
outfiletext=${3:-"phoseedtime_0_SRPlot"}
dir=${4:-"plots/ntuples_v4/checks_v3/kF_v2"}

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
for label in "" "_Scaled_GJets" "_Scaled_QCD"
do
    cp ${outfiletext}${label}_log.png ${outfiletext}${label}_lin.png ${outfiletext}${label}.root ${outfiletext}${label}_integrals.txt ${fulldir}
done

## Final message
echo "Finished SRPlotting for:" ${srplotconfig}
