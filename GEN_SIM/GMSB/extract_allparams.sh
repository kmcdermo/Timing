#!/bin/bash

for lamb in 100 150 200 250 300 350 400
do
    for ctau in 0.1 10 200 400 600 800 1000 1200
    do
	slha=SLHA/GMSB_Lambda${lamb}TeV_CTau${ctau}cm.slha
	mass=$(grep -m 1 "1000022" ${slha} | cut -d ' ' -f9)
	br=$(grep "Z1SS   -->  GVSS   GM" ${slha} | cut -d ' ' -f7)
	width=$(grep "DECAY   1000022" ${slha} | cut -d ' ' -f6)
	echo ${lamb} ${ctau} ${mass} ${width} ${br} >> all_params.txt
    done
done
