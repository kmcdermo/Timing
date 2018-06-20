#!/bin/bash

infilename=${1:-"jetphi_0_control_gjets_DoubleEG.root"}
rescaleconfig=${2:-"rescale_config/qcdMC_to_gjetsMC.txt"}
plotconfig=${3:-"plot_config/jetphi_0.txt"}
miscconfig=${4:-"misc_config/misc.txt"}
outfiletext=${5:-"rescaled_jetphi_0"}
dir=${6:-"plots/ntuples_v4/test"}

## first make plot
root -l -b -q runRescalePlotter.C\(\"${infilename}\",\"${rescaleconfig}\",\"${plotconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

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
echo "Finished RescalePlotting for:" ${infilename}
