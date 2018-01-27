#!/bin/bash

infilename=${1:-"met_vs_time.root"}
outfilename=${2:-"fitresults.root"}
dir=${3:-"fits"}

root -b -q -l runFitter.C\(\"${infilename}\",\"${outfilename}\"\)

## copy out
topdir=/afs/cern.ch/user/k/kmcdermo/www/dispho
fulldir=${topdir}/${dir}

mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${dir}
popd

cp xfit_2D.png yfit_2D.png fit_projX.png fit_projY.png ${outfilename} ${fulldir}
