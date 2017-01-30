#!/bin/sh

#ecal accept only
for cut in cuts #nocuts #cuts
do
    for ctau in 100 2000 6000
    do
	root -l -b -q "runPhotonPlots_signal.C("\"${cut}\",\"${ctau}\",0,0")"
    done
done

# isEB
# for reco in withReReco #noReReco
# do
#     for cut in cuts #nocuts #cuts
#     do
# 	for ctau in 100 2000 6000
# 	do
# 	    root -l -b -q "runPhotonPlots_signal.C("\"${cut}\",\"${ctau}\",\"${reco}\",1,0")"
# 	done
#     done
# done

# isEE
# for reco in withReReco #noReReco
# do
#     for cut in cuts #nocuts #cuts
#     do
# 	for ctau in 100 2000 6000
# 	do
# 	    root -l -b -q "runPhotonPlots_signal.C("\"${cut}\",\"${ctau}\",\"${reco}\",0,1")"
# 	done
#     done
# done