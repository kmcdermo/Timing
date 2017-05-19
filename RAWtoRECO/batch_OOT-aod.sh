#! /bin/sh 

run_dir=/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_0_26_patch2/src/
ana_dir=/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_0_26_patch2/src/Timing/RAWtoRECO

cd $run_dir
eval `scram runtime -sh`
cd $ana_dir

echo "start running job"
cmsRun OOT-RAW2DIGI_L1Reco_RECO-AOD.py
echo "end running job"