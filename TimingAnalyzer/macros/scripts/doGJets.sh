#!/bin/sh

for cut in nocuts cuts
do
    for HT in 40To100 100To200 200To400 400To600 600ToInf
    do
	root -l -b -q "runPhotonPlots_HT.C("\"${cut}\",\"GJets\",\"${HT}\"")"
    done
done
