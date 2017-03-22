#!/bin/sh

vid1=${1:-"medium"}
apply=${2:-1}
phmc=${3:-1}

# inclusive
for ctau in 100 1000
do
    root -l -b -q "runPhotonPlots_hvds.C("\"${vid1}\",\"${ctau}\",${apply},0,0,${phmc}")"
done

# isEB
for ctau in 100 1000
do
    root -l -b -q "runPhotonPlots_hvds.C("\"${vid1}\",\"${ctau}\",${apply},1,0,${phmc}")"
done

# isEE
for ctau in 100 1000
do
    root -l -b -q "runPhotonPlots_hvds.C("\"${vid1}\",\"${ctau}\",${apply},0,1,${phmc}")"
done
