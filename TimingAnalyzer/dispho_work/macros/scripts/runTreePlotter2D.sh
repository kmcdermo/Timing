#!/bin/bash

infilename=${1:-"test.root"}
insignalfilename=${2:-"signals.root"}
cutconfig=${3:-"cut_config/signal_blinded.txt"}
varwgtmapconfig=${4:-"varwgt_config/empty.txt"}
plotconfig=${5:-"plot_config/met_vs_time.txt"}
miscconfig=${6:-"misc_config/blind2D.txt"}
outfiletext=${7:-"met_vs_time.root"}
dir=${8:-"plots2D"}

## run tree plotter first
root -l -b -q runTreePlotter2D.C\(\"${infilename}\",\"${insignalfilename}\",\"${cutconfig}\",\"${varwgtmapconfig}\",\"${plotconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

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

## Final message
echo "Finished TreePlotting2D for plot:" ${plotconfig}
