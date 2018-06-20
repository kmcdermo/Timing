#!/bin/bash

infilename=${1:-"signal_skims.root"}
outtext=${2:-"signal_efficiency"}
dir=${3:-"plots"}

root -l -b -q computeSignalEfficiency.C\(\"${infilename}\",\"${outtext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

## copy everything
cp ${outtext}.png ${fulldir}

## Final message
echo "Finished ComputingSignalEfficiency"
