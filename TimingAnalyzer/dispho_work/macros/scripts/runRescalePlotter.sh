#!/bin/bash

infilename=${1:-"jetphi_0_control_gjets_DoubleEG.root"}
rescaleconfig=${2:-"rescale_config/qcdMC_to_gjetsMC.txt"}
miscconfig=${3:-"plot_config/misc.txt"}
outfiletext=${4:-"rescaled"}
dir=${5:-"plots/dispho/ntuples_v4"}

## first make plot
root -l -b -q runRescalePlotter.C\(\"${infilename}\",\"${rescaleconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

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
