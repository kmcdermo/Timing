#!/bin/sh

for phpt in 20 35 50 80
do
    mkdir -p output/phpt${phpt}
    for phvid in 1 2 
    do
	. /Applications/root_v6.06.08/bin/thisroot.sh
	mkdir -p output/phpt${phpt}/phvid${phvid}
	root -l -b -q "effPlot.C("${phpt},${phvid}")"
	. /Applications/root_v5.34.36/bin/thisroot.sh
	for jetpt in 20 30 40 50
	do
	    for njets in 1 2 3
	    do
		root -l -b -q "runSkim.C("${phpt},${phvid},${jetpt},${njets},1")"
	    done
	done
    done
done

for phpt in 20 35 50 80
do
    for phvid in 1 2 
    do
	root -l -b -q "runSkim.C("${phpt},${phvid},0,0,1")"
    done
done

. /Applications/root_v6.06.08/bin/thisroot.sh

for phpt in 20 35 50 80
do
    for phvid in 1 2 
    do
	root -l -b -q "effStackPlot.C("${phpt},${phvid}")"
    done
done