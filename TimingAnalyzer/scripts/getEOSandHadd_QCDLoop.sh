#!/bin/bash

bin=${1}
base=${2}

qtune="EMEnriched_TuneCUETP8M1_13TeV_pythia8"

for range in "15to20" "20to30" "30to50" "50to80" "80to120" "120to170" "170to300"
do
    mkdir -p Pt-${bin}    
    path=/store/user/kmcdermo/QCD_Pt-${bin}_${qtune}/${base}_QCD_Pt-${bin}_${qtune}
    timestamp=$(eos ls ${path})
    xrdcp -r root://eoscms/${path}/${timestamp}/0000/ Pt-${bin}
done

hadd -O -k Pt-${bin}/tree-${bin}.root Pt-${bin}/dispho_*.root
rm -rf Pt-${bin}/dispho_*.root
