#!/bin/bash

for lamb in 100 150 200 250 300 350 400
do
    for ctau in 0.1 10 200 400 600 800 1000 1200
    do
	c_grav=$(grep "c_grav" GMSB_Lambda${lamb}TeV_CTau${ctau}cm.slha | cut -d ' ' -f11) # re-interpolate
	width=$(grep "DECAY   1000022" GMSB_Lambda${lamb}TeV_CTau${ctau}cm.slha | cut -d ' ' -f6)
	echo ${c_grav} ${lamb} ${width} >> width_cgrav.txt
    done
done
