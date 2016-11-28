#!/bin/sh

for reco in noReReco withReReco
do
    for cut in nocuts cuts
    do
	for ctau in 
	do
	    root -l -b -q "runPhotonPlots_signal.C("\"${cut}\",\"${ctau}\",\"${reco}\"")"
	done
    done
done