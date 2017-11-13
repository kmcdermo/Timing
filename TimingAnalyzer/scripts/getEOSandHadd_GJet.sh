#!/bin/bash

base=${1}

gtune="DoubleEMEnriched_13TeV_pythia8"
dir=tmp

path=/store/user/kmcdermo/GJet_${gtune}/${base}_GJet_${gtune}
timestamp=$(eos ls ${path})
xrdcp -r root://eoscms/${path}/${timestamp}/0000/ ${dir}

hadd -O -k tree.root ${dir}/*.root
rm -rf ${dir}/*.root
