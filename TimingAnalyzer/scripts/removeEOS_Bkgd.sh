#!/bin/bash

base=${1}

# QCD
qtune="EMEnriched_TuneCUETP8M1_13TeV_pythia8"
for range in "15to20" "20to30" "30to50" "50to80" "80to120" "120to170" "170to300"
do
    eos rm -r /store/user/kmcdermo/QCD_Pt-${range}_${qtune}/${base}_QCD_Pt-${range}_${qtune}
done

# GJets
gtune="DoubleEMEnriched_13TeV_pythia8"
eos rm -r /store/user/kmcdermo/GJet_${gtune}/${base}_GJet_${gtune}
