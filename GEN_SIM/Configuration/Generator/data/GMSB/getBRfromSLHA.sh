#!/bin/sh

for lambda in 100 150 200 250 300 350 400
do
    for ctau in 0.1 10 200 400 600 800 1000 1200
    do
	BR=$(grep "Z1SS   -->  GVSS   GM" GMSB_Lambda${lambda}TeV_CTau${ctau}cm.slha | cut -d " " -f 7)
	echo ${lambda} ${ctau} ${BR} >> gmsbBRs.txt
    done
done
