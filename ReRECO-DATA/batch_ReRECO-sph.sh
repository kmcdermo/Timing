#! /bin/sh 

ana_dir=/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_0_26_patch2/src/Timing/ReRECO-DATA/
run_dir=/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_0_26_patch2/src/

cd $run_dir
eval `scram runtime -sh`
cd $ana_dir

echo "start running job"
cmsRun ReRECO_RECO-sph.py
echo "end running job"