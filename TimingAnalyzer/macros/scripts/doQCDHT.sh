#!/bin/sh

#ecal accept only
# for cut in cuts
# do
#     for HT in 100To200 200To300 300To500 500To700 700To1000 1000To1500 1500To2000 2000ToInf
#     do
# 	root -l -b -q "runPhotonPlots_HT.C("\"${cut}\",\"QCD\",\"${HT}\",0,0")"
#     done
# done

# isEB
for vid in none loose
do
    for HT in 100To200 200To300 300To500 500To700 700To1000 1000To1500 1500To2000 2000ToInf
    do
	root -l -b -q "runPhotonPlots_HT.C("\"${vid}\",\"QCD\",\"${HT}\",1,0")"
    done
done

# isEE
for vid in none loose
do
    for HT in 100To200 200To300 300To500 500To700 700To1000 1000To1500 1500To2000 2000ToInf
    do
	root -l -b -q "runPhotonPlots_HT.C("\"${vid}\",\"QCD\",\"${HT}\",0,1")"
    done
done

