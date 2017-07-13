#!/bin/sh

for mzp in 300 500 800 1000
do
    for mdp in 20 40 60
    do
	for ctau in 10 50 100 250 500 1000
	do
	    echo "HVDS_MZp-${mzp}_MDP-${mdp}_Ctau-${ctau}_13TeV-pythia8 100000 https://github.com/kmcdermo/Timing/blob/9XY/GEN_SIM/Configuration/GenProduction/python/ThirteenTeV/HVDS/HVDS_MZP${mzp}_MDP${mdp}_Ctau${ctau}_Pythia8_13TeV_cff.py Pythia8" >> hvds.csv
	done
    done
done