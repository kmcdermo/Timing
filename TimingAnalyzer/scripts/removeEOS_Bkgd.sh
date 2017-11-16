#!/bin/bash

base=${1}

# QCD HT
qcdtune="TuneCUETP8M1_13TeV-madgraphMLM-pythia8"
for bin in 100to200 200to300 300to500 500to700 700to1000 1000to1500 1500to2000 2000toInf
do
    dataset=QCD_HT${bin}
    eos rm -r /store/user/kmcdermo/${dataset}_${qcdtune}/${base}_${dataset}_${qcdtune}
done

# GJets HT
gjetstune="TuneCUETP8M1_13TeV-madgraphMLM-pythia8"
for bin in 40To100 100To200 200To400 400To600 600ToInf
do
    dataset=GJets_HT-${bin}
    eos rm -r /store/user/kmcdermo/${dataset}_${gjetstune}/${base}_${dataset}_${gjetstune}
done
