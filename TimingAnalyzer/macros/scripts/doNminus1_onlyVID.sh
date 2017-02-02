#!/bin/sh

for njets in 1
do
    mkdir -p output/recoskim/njets${njets}
    for jetpt in 30 50
    do
	. /Applications/root_v6.06.08/bin/thisroot.sh
	mkdir -p output/recoskim/onlyvid-sph1/njets${njets}/jetpt${jetpt}
	root -l -b -q "effPlot_onlyVID.C("${jetpt},${njets}")"
	. /Applications/root_v5.34.36/bin/thisroot.sh
	for phpt in 30 40 50
	do
	    for only in none PhIso NeuIso ChgIso Sieie HoE Sminor ClShape ClShapeP
	    do
		root -l -b -q "runSkim_onlyVID.C("${phpt},\"${only}\",${jetpt},${njets},1")"
	    done
	done
    done
done

. /Applications/root_v6.06.08/bin/thisroot.sh
for njets in 1
do
    for jetpt in 30 50
    do
 	root -l -b -q "effStackPlot_onlyVID.C("${jetpt},${njets}")"
    done
done

# njets == 0, no loop over jet pt
. /Applications/root_v6.06.08/bin/thisroot.sh
mkdir -p output/recoskim/onlyvid-sph1/njets0/jetpt0
root -l -b -q "effPlot_onlyVID.C("0,0")"
. /Applications/root_v5.34.36/bin/thisroot.sh
for phpt in 30 40 50
do
    for only in none PhIso NeuIso ChgIso Sieie HoE Sminor ClShape ClShapeP
    do
	root -l -b -q "runSkim_onlyVID.C("${phpt},\"${only}\",0,0,1")"
    done
done

# njets == 0, no range of pt
. /Applications/root_v6.06.08/bin/thisroot.sh
root -l -b -q "effStackPlot_onlyVID.C("0,0")"
