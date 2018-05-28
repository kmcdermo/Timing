#!/bin/bash

indir=${1:-"output"}
infilename=${2:-"AsymLim"}
doobserved=${3:-0}
outtext=${4:-"limit2D"}
dir=${5:-"plots/ntuples_v4/limits"}

root -l -b -q runLimits2D.C\(\"${indir}\",\"${infilename}\",${doobserved},\"${outtext}\"\)

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
