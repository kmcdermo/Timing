#!/bin/bash

for lambda in 100 150 200 250 300 350 400
do 
    for ctau in 600 1200
    do
	crab status -d crab/crab_GMSB_L${lambda}TeV_CTau${ctau}cm_step0
    done
done