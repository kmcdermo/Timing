#!/bin/sh

for lambda in 100 150 200 250 300 350 400 500 600
do
    for ctau in 0.001 0.01 0.1 0.5 1 5 10 50 100 200 400 600 800 1000 1200 5000 10000
    do
	sed "s/XXX/${lambda}/" <GMSB_LXXXTeV_CtauYYYcm_Pythia8_13TeV_cff.py >GMSB_L${lambda}TeV_Ctau${ctau}cm_Pythia8_13TeV_cff.py
	sed -i '' "s/YYY/${ctau}/" GMSB_L${lambda}TeV_Ctau${ctau}cm_Pythia8_13TeV_cff.py
    done
done
