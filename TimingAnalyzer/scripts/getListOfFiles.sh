#!/bin/bash

label=${1}
outdir=${2:-"files"}

mkdir -p ${outdir}

## Data first
dataset="SinglePhoton"
for subera in "B v1" "B v2" "C v1" "C v2" "C v3" "D v1" "E v1" "F v1"
do echo ${subera} | while read -r era version
    do
	./scripts/getListOfFilesData.sh ${label} ${outdir} ${dataset} ${era} ${version}
    done
done

## QCD HT
qcdtune="TuneCUETP8M1_13TeV-madgraphMLM-pythia8"
for bin in 100to200 200to300 300to500 500to700 700to1000 1000to1500 1500to2000 2000toInf
do
    dataset=QCD_HT${bin}
    ./scripts/getListOfFilesMC.sh ${label}_Bkgd ${outdir} ${dataset} ${qcdtune}
done

## GJets HT
gjetstune="TuneCUETP8M1_13TeV-madgraphMLM-pythia8"
for bin in 40To100 100To200 200To400 400To600 600ToInf
do
    dataset=GJets_HT-${bin}
    ./scripts/getListOfFilesMC.sh ${label}_Bkgd ${outdir} ${dataset} ${gjetstune}
done

## GMSB
dataset=GMSB
gmsbtune="L200TeV_CTau400cm_930"
./scripts/getListOfFilesMC.sh ${label}_Signal ${outdir} ${dataset} ${gmsbtune}
