#!/bin/sh

for lambda in 100 150 200 250 300 350 400
do
    for ctau in 0.1 10 200 400 600 800 1000 1200
    do
	sed "s/XXX/${lambda}/" <GMSB_LXXXTeV_CtauYYYcm_Pythia8_13TeV_cff.py >GMSB_L${lambda}TeV_Ctau${ctau}cm_Pythia8_13TeV_cff.py
	sed -i '' "s/YYY/${ctau}/" GMSB_L${lambda}TeV_Ctau${ctau}cm_Pythia8_13TeV_cff.py
    done
done
