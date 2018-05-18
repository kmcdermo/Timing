#!/bin/bash

indir=${1:-"output"}
infilename=${2:-"AsymLim"}
outtext=${3:-"limit1D"}
dir=${4:-"plots/limits"}

root -l -b -q runLimits1D.C\(\"${indir}\",\"${infilename}\",\"${outtext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

## copy everything
cp ${outtext}*.png ${outtext}*.pdf ${outtext}.root ${fulldir}
