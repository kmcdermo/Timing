#!/bin/sh

# isEB
for vid in none loose
do
    root -l -b -q "runPhotonPlots_hvds.C("\"${vid}\",1,0")"
done

#isEE
for vid in none loose
do
    root -l -b -q "runPhotonPlots_hvds.C("\"${vid}\",0,1")"
done
