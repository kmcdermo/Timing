#!/bin/bash

indir=${1:-"output"}
infilename=${2:-"AsymLim"}
limitconfig=${3:-"limit_config/limits2D.txt"}
outtext=${4:-"limit2D"}
dir=${5:-"plots/ntuples_v4/limits"}

root -l -b -q runLimits2D.C\(\"${indir}\",\"${infilename}\",\"${limitconfig}\",\"${outtext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

## copy everything
for ext in png pdf eps
do
    cp ${outtext}*.${ext} ${fulldir}
done
cp ${outtext}.root ${fulldir}


## Final message
echo "Finished RunningLimits2D"
