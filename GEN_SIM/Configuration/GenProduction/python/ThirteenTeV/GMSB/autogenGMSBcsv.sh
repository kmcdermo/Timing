#!/bin/sh

for lambda in 100 150 200 250 300 350 400
do
    for ctau in 0.1 10 200 400 600 800 1000 1200
    do
	echo "GMSB_L-${lambda}TeV_Ctau-${ctau}cm_13TeV-pythia8,100000,https://github.com/kmcdermo/Timing/blob/9XY/GEN_SIM/Configuration/GenProduction/python/ThirteenTeV/GMSB/GMSB_L${lambda}TeV_Ctau${ctau}cm_Pythia8_13TeV_cff.py,Pythia8,https://github.com/kmcdermo/Timing/blob/9XY/GEN_SIM/Configuration/Generator/data/GMSB/GMSB_Lambda${lambda}TeV_CTau${ctau}cm.slha" >> gmsb.csv
    done
done
