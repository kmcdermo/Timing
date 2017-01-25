#!/bin/sh

#ecal accept only
for cut in cuts
do
    for HT in 40To100 100To200 200To400 400To600 600ToInf
    do
	root -l -b -q "runPhotonPlots_HT.C("\"${cut}\",\"GJets\",\"${HT}\",0,0")"
    done
done

#isEB
# for cut in cuts #nocuts cuts
# do
#     for HT in 40To100 100To200 200To400 400To600 600ToInf
#     do
# 	root -l -b -q "runPhotonPlots_HT.C("\"${cut}\",\"GJets\",\"${HT}\",1,0")"
#     done
# done

#isEE
# for cut in nocuts cuts
# do
#     for HT in 40To100 100To200 200To400 400To600 600ToInf
#     do
# 	root -l -b -q "runPhotonPlots_HT.C("\"${cut}\",\"GJets\",\"${HT}\",0,1")"
#     done
# done
