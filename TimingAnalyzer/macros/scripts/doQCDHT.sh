#!/bin/sh

for cut in nocuts cuts
do
    for HT in 100To200 200To300 300To500 500To700 700To1000 1000To1500 1500To2000 2000ToInf
    do
	root -l -b -q "runPhotonPlots_HT.C("\"${cut}\",\"QCD\",\"${HT}\"")"
    done
done
