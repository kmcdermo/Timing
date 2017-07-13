#!/bin/sh

for mzp in 300 500 800 1000
do
    for mdp in 20 40 60
    do
	for ctau in 1 100 500 1000 2500 5000 10000
	do
	    sed "s/4900023:m0 = XXX/4900023:m0 = ${mzp}/" <HVDS_MZPXXX_MDPYYY_CtauZZZmm_Pythia8_13TeV_cff.py >HVDS_MZP${mzp}_MDP${mdp}_Ctau${ctau}mm_Pythia8_13TeV_cff.py
	    sed -i '' "s/4900111:m0 = YYY/4900111:m0 = ${mdp}/" HVDS_MZP${mzp}_MDP${mdp}_Ctau${ctau}mm_Pythia8_13TeV_cff.py
	    sed -i '' "s/4900111:tau0 = ZZZ/4900111:tau0 = ${ctau}/" HVDS_MZP${mzp}_MDP${mdp}_Ctau${ctau}mm_Pythia8_13TeV_cff.py
	done
    done
done
