#!/bin/sh

# isEB
for vid in none medium
do
    for ctau in 100 2000 6000
    do
	root -l -b -q "runPhotonPlots_gmsb.C("\"${vid}\",\"${ctau}\",1,0")"
    done
done

#isEE
for vid in none medium
do
    for ctau in 100 2000 6000
    do
	root -l -b -q "runPhotonPlots_gmsb.C("\"${vid}\",\"${ctau}\",0,1")"
    done
done
