#!/bin/sh

vid1=${1:-"medium"}
apply=${2:-1}
phmc=${3:-1}

# inclusive
for HT in 100To200 200To300 300To500 500To700 700To1000 1000To1500 1500To2000 2000ToInf
do
    root -l -b -q "runPhotonPlots_HT.C("\"${vid1}\",\"QCD\",\"${HT}\",${apply},0,0,0,1,${phmc}")"
done

# isEB
for HT in 100To200 200To300 300To500 500To700 700To1000 1000To1500 1500To2000 2000ToInf
do
    root -l -b -q "runPhotonPlots_HT.C("\"${vid1}\",\"QCD\",\"${HT}\",${apply},1,0,0,1,${phmc}")"
done

# isEE
for HT in 100To200 200To300 300To500 500To700 700To1000 1000To1500 1500To2000 2000ToInf
do
    root -l -b -q "runPhotonPlots_HT.C("\"${vid1}\",\"QCD\",\"${HT}\",${apply},0,1,0,1,${phmc}")"
done
