#!/bin/bash

outname=${1:-"skimmedtree.root"}
eosbase=${2:-"root://eoscms//store/user/kmcdermo/nTuples/2017"}

## Data first
#for subera in "B v1" "B v2" "C v1" "C v2" "C v3" "D v1" "E v1" "F v1"
#do echo ${subera} | while read -r era version
#    do
#    done
#done

dir="Data/singleph"
root -l -b -q macros/skimDisPho.C\(\"${eosbase}/${dir}\",\"${outname}\"\)
xrdcp ${outname} "${eosbase}/${dir}"

## QCD second
for bin in "15to20" "20to30" "30to50" "50to80" "80to120" "120to170" "170to300"
do
    dir="MC/qcd/Pt-${bin}"
    root -l -b -q macros/skimDisPho.C\(\"${eosbase}/${dir}\",\"${outname}\"\)
    xrdcp ${outname} "${eosbase}/${dir}/"
done

## GJets
dir="MC/gjets-EM"
root -l -b -q macros/skimDisPho.C\(\"${eosbase}/${dir}\",\"${outname}\"\)
xrdcp ${outname} "${eosbase}/${dir}/"

## GMSB
dir="MC/gmsb"
root -l -b -q macros/skimDisPho.C\(\"${eosbase}/${dir}\",\"${outname}\",0\)
xrdcp ${outname} "${eosbase}/${dir}/"
