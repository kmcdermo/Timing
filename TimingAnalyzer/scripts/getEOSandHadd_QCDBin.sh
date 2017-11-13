#!/bin/bash

bin=${1}
base=${2}

qtune="EMEnriched_TuneCUETP8M1_13TeV_pythia8"

mkdir -p Pt-${bin}
path=/store/user/kmcdermo/QCD_Pt-${bin}_${qtune}/${base}_QCD_Pt-${bin}_${qtune}
timestamp=$(eos ls ${path})
xrdcp -r root://eoscms/${path}/${timestamp}/0000/ Pt-${bin}

hadd -O -k Pt-${bin}/tree-${bin}.root Pt-${bin}/dispho_*.root
rm -rf Pt-${bin}/dispho_*.root

