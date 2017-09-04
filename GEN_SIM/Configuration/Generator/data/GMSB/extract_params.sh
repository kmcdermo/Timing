#!/bin/bash

for lamb in 100 120 140 160 180
do
    for ctau in 1 10 100 1000 2000 3000 4000 6000
    do
	c_grav=$(grep "c_grav" GMSB_Lambda${lamb}_CTau${ctau}.slha | cut -d ' ' -f11) # old files from Livia's GMSB directory
	width=$(grep "DECAY   1000022" GMSB_Lambda${lamb}_CTau${ctau}.slha | cut -d ' ' -f6)
	echo ${c_grav} ${lamb} ${width} >> width_cgrav.txt
    done
done
