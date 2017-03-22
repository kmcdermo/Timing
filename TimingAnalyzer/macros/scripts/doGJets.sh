#!/bin/sh

vid1=${1:-"medium"}
apply=${2:-1}
phmc=${3:-1}

# inclusive
for HT in 40To100 100To200 200To400 400To600 600ToInf
do
    root -l -b -q "runPhotonPlots_HT.C("\"${vid1}\",\"GJets\",\"${HT}\",${apply},0,0,1,0,${phmc}")"
done

# isEB
for HT in 40To100 100To200 200To400 400To600 600ToInf
do
    root -l -b -q "runPhotonPlots_HT.C("\"${vid1}\",\"GJets\",\"${HT}\",${apply},1,0,1,0,${phmc}")"
done

# isEE
for HT in 40To100 100To200 200To400 400To600 600ToInf
do
    root -l -b -q "runPhotonPlots_HT.C("\"${vid1}\",\"GJets\",\"${HT}\",${apply},0,1,1,0,${phmc}")"
done

