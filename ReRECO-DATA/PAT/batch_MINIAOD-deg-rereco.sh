#! /bin/sh 

ana_dir=/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_0_26_patch2/src/Timing/ReRECO-DATA/PAT/
run_dir=/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_0_26_patch2/src/

cd $run_dir
eval `scram runtime -sh`
cd $ana_dir

echo "start running job"
cmsRun step3_PAT_rereco_deg.py
echo "end running job"